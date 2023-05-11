using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ic4.Examples
{
    public static class WinformsUtil
    {
        private static bool isInitialized_ = false;
        static WinformsUtil()
        {
            Scaling = 1.0f;
        }

        public static void Init(Control ctrl)
        {
            if (!isInitialized_)
            {
                using (var g = ctrl.CreateGraphics())
                {
                    DPI = g.DpiX;
                    Scaling = g.DpiX / 96.0f;
                }
                isInitialized_ = true;
            }
        }

        public static float DPI { get; private set; }
        public static float Scaling { get; set; }

        public static class NativeMethods
        {
            public const int WM_SETREDRAW = 11;
            public const Int32 CB_SETITEMHEIGHT = 0x153;

            [DllImport("uxtheme.dll", CharSet = CharSet.Unicode)]
            public extern static int SetWindowTheme(IntPtr hWnd, string pszSubAppName, string pszSubIdList);

            [DllImport("user32.dll")]
            public static extern IntPtr SendMessage(IntPtr hWnd, int msg, IntPtr wp, IntPtr lp);

            [DllImport("user32.dll")]
            public static extern int SendMessage(IntPtr hWnd, Int32 wMsg, bool wParam, Int32 lParam);
        }

        public static void SuspendDrawing(Control parent)
        {
            NativeMethods.SendMessage(parent.Handle, NativeMethods.WM_SETREDRAW, false, 0);
        }
        public static void ResumeDrawing(Control parent)
        {
            NativeMethods.SendMessage(parent.Handle, NativeMethods.WM_SETREDRAW, true, 0);
            parent.Refresh();
        }
    }
}
