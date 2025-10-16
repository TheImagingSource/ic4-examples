using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DoLP_Segmentation.PixelTypes
{
    public struct PolarizedADIMono8
    {
        public byte AoLP;
        public byte DoLP;
        public byte Intensity;
        public byte Reserved;
    }
}
