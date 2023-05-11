using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ic4.Examples
{
    internal static class Appearance
    {
        public static readonly Color Color = new Color();

        public static int Indentation = (int)(16.0f * WinformsUtil.Scaling);
        public static int Spacing = (int)(0.0f * WinformsUtil.Scaling);
        public static int HeaderRowHeight = (int)(32.0f * WinformsUtil.Scaling);
        public static int ControlRowHeight = (int)(40.0f * WinformsUtil.Scaling);
        public static int TextSpacing = (int)(8.0f * WinformsUtil.Scaling);
        public static int MaximumControlWidth = (int)(320.0f * WinformsUtil.Scaling);

        // all fonts in GraphicsUnit.Point, so they scale automatically
        public static float NodeFontSize = 8.5f;
        public static float InfoBoxFontSize = 8.5f;
        public static float ControlFontSize = 8.5f;
    }

}
