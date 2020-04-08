using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace microbitmouse
{
    class Program
    {
        #region CONFIGURATION VARIABLES
        private const double
            MouseSensitivity = 1.5;
        private const int
            SMAFactor = 50;
        #endregion

        private static SerialPort port;

        [STAThread]
        static void Main(string[] args)
        {
            while (port == null)
            {
                try
                {
                    Console.Clear();
                    string portName = null;
                    string[] portNames = SerialPort.GetPortNames();
                    if (portNames.Length == 0)
                    {
                        Console.WriteLine("No compatible devices found. Press any key to retry...");
                        Console.ReadLine();
                    }
                    else if (portNames.Length == 1)
                    { portName = portNames[0]; }
                    else if (portNames.Length > 1)
                    {
                        Console.WriteLine("Please pick a COM-port to listen to:");
                        foreach (string p in portNames)
                            Console.Write(p + " ");
                        portName = Console.ReadLine();
                    }
                    if (portName != null)
                    {
                        port = new SerialPort(portName,
                        115200, Parity.None, 8, StopBits.One);
                        port.Open();
                    }
                }
                catch { }
            }

            List<SMAVal> SMA = new List<SMAVal>();
            while (true)
            {
                try
                {
                    if (port.BytesToRead > 0)
                    {
                        //Get buffer
                        byte[] buff = new byte[port.ReadBufferSize];
                        port.Read(buff, 0, buff.Length);

                        //Parse and prepare data
                        string data = Encoding.ASCII.GetString(buff);
                        if (data.Count(f => f == '(') == 1 &&
                            data.Count(f => f == ')') == 1)
                        {
                            data = data.TrimStart('\0', '\n', '\r');
                            data = data.TrimEnd('\0', '\n', '\r');
                            string[] dataset = data.Substring(1, data.Length - 2).Split(',');
                            int x = Int32.Parse(dataset[0]),
                                y = Int32.Parse(dataset[1]);
							bool l = dataset[2][0] == '0' ? false : true,
                                r = dataset[3][0] == '0' ? false : true;

                            //Check if Int32.TryParse() resulted in anything
                            if (x != 0 && y != 1)
                            {
                                //Add to running avg.
                                SMAVal xy = new SMAVal(x, y);
                                SMA.Add(xy);
                                //Calculate moving average
                                int
                                    factor = SMAFactor,
                                    x_sum = 0,
                                    y_sum = 0,
                                    x_SMA = 0,
                                    y_SMA = 0;

                                for (int i = 0; i < SMA.Count; i++)
                                {
                                    if (i >= factor)
                                    {
                                        x_sum -= SMA[i - factor].x;
                                        y_sum -= SMA[i - factor].y;
                                    }

                                    x_sum += SMA[i].x;
                                    y_sum += SMA[i].y;

                                    x_SMA = (int)(x_sum / factor);
                                    y_SMA = (int)(y_sum / factor);
                                }

                                //Remap the accelerometer values to locations on the display
                                int rx = map(x_SMA, -1024, 1024, 0, 1920),
                                    ry = map(y_SMA, -1024, 1024, 0, 1080);
                                //Log
                                //Console.WriteLine("{0}, {1}:{2}", rx, ry, y);
                                //Move cursor
                                SetCursorPos(rx, ry);
								
								if(l)
									mouse_event((int)MouseEventFlags.LEFTDOWN, rx, ry, 0, 0);
								if(r)
									mouse_event((int)MouseEventFlags.RIGHTDOWN, rx, ry, 0, 0);
								//mouse_event((int)MouseEventFlags.ABSOLUTE, rx, ry, 0, 0);
                            }
                        }
                    }
                }
                catch (Exception e) { 
					Console.WriteLine(e.Message);
				}
                Thread.Sleep(2);
            }
        }

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool SetCursorPos(int x, int y);
		[DllImport("user32.dll")]
		static extern void mouse_event(int dwFlags, int dx, int dy, int cButtons, int dwExtraInfo);
		
		[Flags]
		public enum MouseEventFlags
		{
			LEFTDOWN = 0x00000002,
			LEFTUP = 0x00000004,
			MIDDLEDOWN = 0x00000020,
			MIDDLEUP = 0x00000040,
			MOVE = 0x00000001,
			ABSOLUTE = 0x00008000,
			RIGHTDOWN = 0x00000008,
			RIGHTUP = 0x00000010
		}

        private static int map(double x, int in_min, int in_max, int out_min, int out_max)
        {
            x *= MouseSensitivity;
            return (int)Math.Round((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
        }
    }

    public class SMAVal
    {
        public int
            x,
            y;

        public SMAVal(int x, int y)
        {
            this.x = x;
            this.y = y;
        }
    }
}