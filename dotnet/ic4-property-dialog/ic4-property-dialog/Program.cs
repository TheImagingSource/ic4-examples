using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ic4_property_dialog
{
    internal static class Program
    {
        private static class NativeMethods
        {
            [System.Runtime.InteropServices.DllImport("user32.dll")]
            public static extern bool SetProcessDPIAware();
        }

        static string GetWindowsVersion()
        {
            try
            {

                var registryKey = Registry.LocalMachine.OpenSubKey("Software\\Microsoft\\Windows NT\\CurrentVersion");
                string pathName = (string)registryKey.GetValue("productName");
                return pathName;
            }
            catch
            {
                return Environment.OSVersion.ToString();
            }
        }

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            // Sets the current process as dots per inch (dpi) aware.
            if (Environment.OSVersion.Version.Major >= 6) NativeMethods.SetProcessDPIAware();

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}
