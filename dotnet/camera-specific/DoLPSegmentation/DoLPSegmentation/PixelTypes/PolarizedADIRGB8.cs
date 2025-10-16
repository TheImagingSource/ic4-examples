using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DoLP_Segmentation.PixelTypes
{
    public struct PolarizedADIRGB8
    {
        public byte AoLP;
        public byte DoLPRed;
        public byte DoLPGreen;
        public byte DoLPBlue;
        public byte IntensityRed;
        public byte IntensityGreen;
        public byte IntensityBlue;
        public byte Reserved;
    }
}
