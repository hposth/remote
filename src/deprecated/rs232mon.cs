using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace rs232mon
{
    class Program
    {
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
						data = data.TrimStart('\0', '\n', '\r');
                        data = data.TrimEnd('\0', '\n', '\r');
                        Console.WriteLine("D:" + data);
                    }
                }
                catch (Exception) { }
                Thread.Sleep(1);
            }
        }
    }
}