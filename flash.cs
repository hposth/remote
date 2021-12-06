using System;
using System.IO;
using System.Linq;
using System.Threading;

namespace bootloader
{
    class Program
    {
        static void Main(string[] args)
        {
            DriveInfo[] drives = DriveInfo.GetDrives();

            if (drives.Where<DriveInfo>(D => D.VolumeLabel == "MAINTENANCE").Count() == 1)
            {
                Console.Write("MAINTENANCE MODE FOUND\n");
				Console.Write("PUSHING f.hex >> 0253_kl26z_microbit_0x8000.hex\n");
				
                File.Copy(
                    Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) + @"\f.hex",
                    drives.Where<DriveInfo>(D => D.VolumeLabel == "MAINTENANCE").ToList<DriveInfo>()[0].Name + @"\0253_kl26z_microbit_0x8000.hex");
                Console.Write("AWAITING REBOOT\n");
                while (true)
                {
                    try
                    {
                        if (drives.Where<DriveInfo>(D => D.VolumeLabel == "MICROBIT").Count() == 1)
                            break;
                    }
                    catch (DriveNotFoundException) { }
                    Thread.Sleep(500);
                }
                Console.Write("FIRMWARE FLASH COMPLETE\n");
            }
            Console.Write("MAINTENANCE MODE DISABLED\n");

            Console.Write("PUSHING s.hex >> 0253_kl26z_microbit_0x8000.hex\n");
            File.Copy(
                    Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) + @"\s.hex",
                    drives.Where<DriveInfo>(D => D.VolumeLabel == "MICROBIT").ToList<DriveInfo>()[0].Name + @"\0253_kl26z_microbit_0x8000.hex");
            Console.Write("DONE");
        }
    }
}
