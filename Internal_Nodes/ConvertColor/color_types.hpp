//
// Convert Color - Color Types
//

#ifndef CONVERTCOLOR_COLOR_TYPES_HPP_
#define CONVERTCOLOR_COLOR_TYPES_HPP_
#include <vector>
namespace DSPatch::DSPatchables {
const int ConvertColorCodes[] = {
    /*COLOR_BGR2BGRA = */ 0, //!< add alpha channel to RGB or BGR image
    /*COLOR_RGB2RGBA = */ 0,

    /*COLOR_BGRA2BGR = */1, //!< remove alpha channel from RGB or BGR image
    /*COLOR_RGBA2RGB = */1,

    /*COLOR_BGR2RGBA = */2, //!< convert between RGB and BGR color spaces (with or without alpha channel)
    /*COLOR_RGB2BGRA = */2,

    /*COLOR_RGBA2BGR = */3,
    /*COLOR_BGRA2RGB = */3,

    /*COLOR_BGR2RGB = */4,
    /*COLOR_RGB2BGR = */4,

    /*COLOR_BGRA2RGBA = */5,
    /*COLOR_RGBA2BGRA = */5,

    /*COLOR_BGR2GRAY = */6, //!< convert between RGB/BGR and grayscale, @ref color_convert_rgb_gray "color conversions"
    /*COLOR_RGB2GRAY = */7,
    /*COLOR_GRAY2BGR = */8,
    /*COLOR_GRAY2RGB = */8,
    /*COLOR_GRAY2BGRA = */9,
    /*COLOR_GRAY2RGBA = */9,
    /*COLOR_BGRA2GRAY = */10,
    /*COLOR_RGBA2GRAY = */11,
    /*COLOR_BGR2BGR565 = */12, //!< convert between RGB/BGR and BGR565 (16-bit images)
    /*COLOR_RGB2BGR565 = */13,
    /*COLOR_BGR5652BGR = */14,
    /*COLOR_BGR5652RGB = */15,
    /*COLOR_BGRA2BGR565 = */16,
    /*COLOR_RGBA2BGR565 = */17,
    /*COLOR_BGR5652BGRA = */18,
    /*COLOR_BGR5652RGBA = */19,

    /*COLOR_GRAY2BGR565 = */20, //!< convert between grayscale to BGR565 (16-bit images)
    /*COLOR_BGR5652GRAY = */21,

    /*COLOR_BGR2BGR555 = */22,  //!< convert between RGB/BGR and BGR555 (16-bit images)
    /*COLOR_RGB2BGR555 = */23,
    /*COLOR_BGR5552BGR = */24,
    /*COLOR_BGR5552RGB = */25,
    /*COLOR_BGRA2BGR555 =*/26,
    /*COLOR_RGBA2BGR555 = */27,
    /*COLOR_BGR5552BGRA = */28,
    /*COLOR_BGR5552RGBA = */29,

    /*COLOR_GRAY2BGR555 = */30, //!< convert between grayscale and BGR555 (16-bit images)
    /*COLOR_BGR5552GRAY = */31,

    /*COLOR_BGR2XYZ = */32, //!< convert RGB/BGR to CIE XYZ, @ref color_convert_rgb_xyz "color conversions"
    /*COLOR_RGB2XYZ = */33,
    /*COLOR_XYZ2BGR = */34,
    /*COLOR_XYZ2RGB = */35,

    /*COLOR_BGR2YCrCb = */36, //!< convert RGB/BGR to luma-chroma (aka YCC), @ref color_convert_rgb_ycrcb "color conversions"
    /*COLOR_RGB2YCrCb = */37,
    /*COLOR_YCrCb2BGR = */38,
    /*COLOR_YCrCb2RGB = */39,

    /*COLOR_BGR2HSV = */40, //!< convert RGB/BGR to HSV (hue saturation value), @ref color_convert_rgb_hsv "color conversions"
    /*COLOR_RGB2HSV = */41,

    /*COLOR_BGR2Lab = */44, //!< convert RGB/BGR to CIE Lab, @ref color_convert_rgb_lab "color conversions"
    /*COLOR_RGB2Lab = */45,

    /*COLOR_BGR2Luv = */50, //!< convert RGB/BGR to CIE Luv, @ref color_convert_rgb_luv "color conversions"
    /*COLOR_RGB2Luv = */51,
    /*COLOR_BGR2HLS = */52, //!< convert RGB/BGR to HLS (hue lightness saturation), @ref color_convert_rgb_hls "color conversions"
    /*COLOR_RGB2HLS = */53,

    /*COLOR_HSV2BGR = */54, //!< backward conversions to RGB/BGR
    /*COLOR_HSV2RGB = */55,

    /*COLOR_Lab2BGR = */56,
    /*COLOR_Lab2RGB = */57,
    /*COLOR_Luv2BGR = */58,
    /*COLOR_Luv2RGB = */59,
    /*COLOR_HLS2BGR = */60,
    /*COLOR_HLS2RGB = */61,

    /*COLOR_BGR2HSV_FULL = */66, //!<
    /*COLOR_RGB2HSV_FULL = */67,
    /*COLOR_BGR2HLS_FULL = */68,
    /*COLOR_RGB2HLS_FULL = */69,

    /*COLOR_HSV2BGR_FULL = */70,
    /*COLOR_HSV2RGB_FULL = */71,
    /*COLOR_HLS2BGR_FULL = */72,
    /*COLOR_HLS2RGB_FULL = */73,

    /*COLOR_LBGR2Lab = */74,
    /*COLOR_LRGB2Lab = */75,
    /*COLOR_LBGR2Luv = */76,
    /*COLOR_LRGB2Luv = */77,

    /*COLOR_Lab2LBGR = */78,
    /*COLOR_Lab2LRGB = */79,
    /*COLOR_Luv2LBGR = */80,
    /*COLOR_Luv2LRGB = */81,

    /*COLOR_BGR2YUV = */82, //!< convert between RGB/BGR and YUV
    /*COLOR_RGB2YUV = */83,
    /*COLOR_YUV2BGR = */84,
    /*COLOR_YUV2RGB = */85,

    //! YUV 4:2:0 family to RGB
    /*COLOR_YUV2RGB_NV12 = */90,
    /*COLOR_YUV2BGR_NV12 = */91,
    /*COLOR_YUV2RGB_NV21 = */92,
    /*COLOR_YUV2BGR_NV21 = */93,
    /*COLOR_YUV420sp2RGB = */92,
    /*COLOR_YUV420sp2BGR = */93,

    /*COLOR_YUV2RGBA_NV12 = */94,
    /*COLOR_YUV2BGRA_NV12 = */95,
    /*COLOR_YUV2RGBA_NV21 = */96,
    /*COLOR_YUV2BGRA_NV21 = */97,
    /*COLOR_YUV420sp2RGBA = */96,
    /*COLOR_YUV420sp2BGRA = */97,

    /*COLOR_YUV2RGB_YV12 = */98,
    /*COLOR_YUV2BGR_YV12 = */99,
    /*COLOR_YUV2RGB_IYUV = */100,
    /*COLOR_YUV2BGR_IYUV = */101,
    /*COLOR_YUV2RGB_I420 = */100,
    /*COLOR_YUV2BGR_I420 = */101,
    /*COLOR_YUV420p2RGB = */98,
    /*COLOR_YUV420p2BGR = */99,

    /*COLOR_YUV2RGBA_YV12 = */102,
    /*COLOR_YUV2BGRA_YV12 = */103,
    /*COLOR_YUV2RGBA_IYUV = */104,
    /*COLOR_YUV2BGRA_IYUV = */105,
    /*COLOR_YUV2RGBA_I420 = */104,
    /*COLOR_YUV2BGRA_I420 = */105,
    /*COLOR_YUV420p2RGBA = */102,
    /*COLOR_YUV420p2BGRA = */103,

    /*COLOR_YUV2GRAY_420 = */106,
    /*COLOR_YUV2GRAY_NV21 = */106,
    /*COLOR_YUV2GRAY_NV12 = */106,
    /*COLOR_YUV2GRAY_YV12 = */106,
    /*COLOR_YUV2GRAY_IYUV = */106,
    /*COLOR_YUV2GRAY_I420 = */106,
    /*COLOR_YUV420sp2GRAY = */106,
    /*COLOR_YUV420p2GRAY = */106,

    //! YUV 4:2:2 family to RGB
    /*COLOR_YUV2RGB_UYVY = */107,
    /*COLOR_YUV2BGR_UYVY = */108,
    //COLOR_YUV2RGB_VYUY = */109,
    //COLOR_YUV2BGR_VYUY = */110,
    /*COLOR_YUV2RGB_Y422 = */107,
    /*COLOR_YUV2BGR_Y422 = */108,
    /*COLOR_YUV2RGB_UYNV = */107,
    /*COLOR_YUV2BGR_UYNV = */108,

    /*COLOR_YUV2RGBA_UYVY = */111,
    /*COLOR_YUV2BGRA_UYVY = */112,
    //COLOR_YUV2RGBA_VYUY = */113,
    //COLOR_YUV2BGRA_VYUY = */114,
    /*COLOR_YUV2RGBA_Y422 = */111,
    /*COLOR_YUV2BGRA_Y422 = */112,
    /*COLOR_YUV2RGBA_UYNV = */111,
    /*COLOR_YUV2BGRA_UYNV = */112,

    /*COLOR_YUV2RGB_YUY2 = */115,
    /*COLOR_YUV2BGR_YUY2 = */116,
    /*COLOR_YUV2RGB_YVYU = */117,
    /*COLOR_YUV2BGR_YVYU = */118,
    /*COLOR_YUV2RGB_YUYV = */115,
    /*COLOR_YUV2BGR_YUYV = */116,
    /*COLOR_YUV2RGB_YUNV = */115,
    /*COLOR_YUV2BGR_YUNV = */116,

    /*COLOR_YUV2RGBA_YUY2 = */119,
    /*COLOR_YUV2BGRA_YUY2 = */120,
    /*COLOR_YUV2RGBA_YVYU = */121,
    /*COLOR_YUV2BGRA_YVYU = */122,
    /*COLOR_YUV2RGBA_YUYV = */119,
    /*COLOR_YUV2BGRA_YUYV = */120,
    /*COLOR_YUV2RGBA_YUNV = */119,
    /*COLOR_YUV2BGRA_YUNV = */120,

    /*COLOR_YUV2GRAY_UYVY = */123,
    /*COLOR_YUV2GRAY_YUY2 = */124,
    //CV_YUV2GRAY_VYUY    = CV_YUV2GRAY_UYVY,
    /*COLOR_YUV2GRAY_Y422 = */123,
    /*COLOR_YUV2GRAY_UYNV = */123,
    /*COLOR_YUV2GRAY_YVYU = */124,
    /*COLOR_YUV2GRAY_YUYV = */124,
    /*COLOR_YUV2GRAY_YUNV = */124,

    //! alpha premultiplication
    /*COLOR_RGBA2mRGBA = */125,
    /*COLOR_mRGBA2RGBA = */126,

    //! RGB to YUV 4:2:0 family
    /*COLOR_RGB2YUV_I420 = */127,
    /*COLOR_BGR2YUV_I420 = */128,
    /*COLOR_RGB2YUV_IYUV = */127,
    /*COLOR_BGR2YUV_IYUV = */128,

    /*COLOR_RGBA2YUV_I420 = */129,
    /*COLOR_BGRA2YUV_I420 = */130,
    /*COLOR_RGBA2YUV_IYUV = */129,
    /*COLOR_BGRA2YUV_IYUV = */130,
    /*COLOR_RGB2YUV_YV12 = */131,
    /*COLOR_BGR2YUV_YV12 = */132,
    /*COLOR_RGBA2YUV_YV12 = */133,
    /*COLOR_BGRA2YUV_YV12 = */134,

    //! Demosaicing
    /*COLOR_BayerBG2BGR = */46,
    /*COLOR_BayerGB2BGR = */47,
    /*COLOR_BayerRG2BGR = */48,
    /*COLOR_BayerGR2BGR = */49,

    /*COLOR_BayerBG2RGB = */48,
    /*COLOR_BayerGB2RGB = */49,
    /*COLOR_BayerRG2RGB = */46,
    /*COLOR_BayerGR2RGB = */47,

    /*COLOR_BayerBG2GRAY = */86,
    /*COLOR_BayerGB2GRAY = */87,
    /*COLOR_BayerRG2GRAY = */88,
    /*COLOR_BayerGR2GRAY = */89,

    //! Demosaicing using Variable Number of Gradients
    /*COLOR_BayerBG2BGR_VNG = */62,
    /*COLOR_BayerGB2BGR_VNG = */63,
    /*COLOR_BayerRG2BGR_VNG = */64,
    /*COLOR_BayerGR2BGR_VNG = */65,

    /*COLOR_BayerBG2RGB_VNG = */64,
    /*COLOR_BayerGB2RGB_VNG = */65,
    /*COLOR_BayerRG2RGB_VNG = */62,
    /*COLOR_BayerGR2RGB_VNG = */63,

    //! Edge-Aware Demosaicing
    /*COLOR_BayerBG2BGR_EA = */135,
    /*COLOR_BayerGB2BGR_EA = */136,
    /*COLOR_BayerRG2BGR_EA = */137,
    /*COLOR_BayerGR2BGR_EA = */138,

    /*COLOR_BayerBG2RGB_EA = */137,
    /*COLOR_BayerGB2RGB_EA = */138,
    /*COLOR_BayerRG2RGB_EA = */135,
    /*COLOR_BayerGR2RGB_EA = */136,

    //! Demosaicing with alpha channel
    /*COLOR_BayerBG2BGRA = */139,
    /*COLOR_BayerGB2BGRA = */140,
    /*COLOR_BayerRG2BGRA = */141,
    /*COLOR_BayerGR2BGRA = */142,

    /*COLOR_BayerBG2RGBA = */141,
    /*COLOR_BayerGB2RGBA = */142,
    /*COLOR_BayerRG2RGBA = */139,
    /*COLOR_BayerGR2RGBA = */140
};

const std::vector<std::string> ConvertColorNames = {
    "BGR2BGRA",
    "RGB2RGBA",

    "BGRA2BGR",
    "RGBA2RGB",

    "BGR2RGBA",
    "RGB2BGRA",

    "RGBA2BGR",
    "BGRA2RGB",

    "BGR2RGB",
    "RGB2BGR",

    "BGRA2RGBA",
    "RGBA2BGRA",

    "BGR2GRAY",
    "RGB2GRAY",
    "GRAY2BGR",
    "GRAY2RGB",
    "GRAY2BGRA",
    "GRAY2RGBA",
    "BGRA2GRAY",
    "RGBA2GRAY",
    "BGR2BGR565",
    "RGB2BGR565",
    "BGR5652BGR",
    "BGR5652RGB",
    "BGRA2BGR565",
    "RGBA2BGR565",
    "BGR5652BGRA",
    "BGR5652RGBA",

    "GRAY2BGR565",
    "BGR5652GRAY",

    "BGR2BGR555",
    "RGB2BGR555",
    "BGR5552BGR",
    "BGR5552RGB",
    "BGRA2BGR555",
    "RGBA2BGR555",
    "BGR5552BGRA",
    "BGR5552RGBA",

    "GRAY2BGR555",
    "BGR5552GRAY",

    "BGR2XYZ",
    "RGB2XYZ",
    "XYZ2BGR",
    "XYZ2RGB",

    "BGR2YCrCb",
    "RGB2YCrCb",
    "YCrCb2BGR",
    "YCrCb2RGB",

    "BGR2HSV",
    "RGB2HSV",

    "BGR2Lab",
    "RGB2Lab",

    "BGR2Luv",
    "RGB2Luv",
    "BGR2HLS",
    "RGB2HLS",

    "HSV2BGR",
    "HSV2RGB",

    "Lab2BGR",
    "Lab2RGB",
    "Luv2BGR",
    "Luv2RGB",
    "HLS2BGR",
    "HLS2RGB",

    "BGR2HSV_FULL",
    "RGB2HSV_FULL",
    "BGR2HLS_FULL",
    "RGB2HLS_FULL",

    "HSV2BGR_FULL",
    "HSV2RGB_FULL",
    "HLS2BGR_FULL",
    "HLS2RGB_FULL",

    "LBGR2Lab",
    "LRGB2Lab",
    "LBGR2Luv",
    "LRGB2Luv",

    "Lab2LBGR",
    "Lab2LRGB",
    "Luv2LBGR",
    "Luv2LRGB",

    "BGR2YUV",
    "RGB2YUV",
    "YUV2BGR",
    "YUV2RGB",

    //! YUV 4:2:0 family to RGB
    "YUV2RGB_NV12",
    "YUV2BGR_NV12",
    "YUV2RGB_NV21",
    "YUV2BGR_NV21",
    "YUV420sp2RGB",
    "YUV420sp2BGR",

    "YUV2RGBA_NV12",
    "YUV2BGRA_NV12",
    "YUV2RGBA_NV21",
    "YUV2BGRA_NV21",
    "YUV420sp2RGBA",
    "YUV420sp2BGRA",

    "YUV2RGB_YV12",
    "YUV2BGR_YV12",
    "YUV2RGB_IYUV",
    "YUV2BGR_IYUV",
    "YUV2RGB_I420",
    "YUV2BGR_I420",
    "YUV420p2RGB",
    "YUV420p2BGR",

    "YUV2RGBA_YV12",
    "YUV2BGRA_YV12",
    "YUV2RGBA_IYUV",
    "YUV2BGRA_IYUV",
    "YUV2RGBA_I420",
    "YUV2BGRA_I420",
    "YUV420p2RGBA",
    "YUV420p2BGRA",

    "YUV2GRAY_420",
    "YUV2GRAY_NV21",
    "YUV2GRAY_NV12",
    "YUV2GRAY_YV12",
    "YUV2GRAY_IYUV",
    "YUV2GRAY_I420",
    "YUV420sp2GRAY",
    "YUV420p2GRAY",

    //! YUV 4:2:2 family to RGB
    "YUV2RGB_UYVY",
    "YUV2BGR_UYVY",
    //COLOR_YUV2RGB_VYUY = 109,
    //COLOR_YUV2BGR_VYUY = 110,
    "YUV2RGB_Y422",
    "YUV2BGR_Y422",
    "YUV2RGB_UYNV",
    "YUV2BGR_UYNV",

    "YUV2RGBA_UYVY",
    "YUV2BGRA_UYVY",
    //COLOR_YUV2RGBA_VYUY = 113,
    //COLOR_YUV2BGRA_VYUY = 114,
    "YUV2RGBA_Y422",
    "YUV2BGRA_Y422",
    "YUV2RGBA_UYNV",
    "YUV2BGRA_UYNV",

    "YUV2RGB_YUY2",
    "YUV2BGR_YUY2",
    "YUV2RGB_YVYU",
    "YUV2BGR_YVYU",
    "YUV2RGB_YUYV",
    "YUV2BGR_YUYV",
    "YUV2RGB_YUNV",
    "YUV2BGR_YUNV",

    "YUV2RGBA_YUY2",
    "YUV2BGRA_YUY2",
    "YUV2RGBA_YVYU",
    "YUV2BGRA_YVYU",
    "YUV2RGBA_YUYV",
    "YUV2BGRA_YUYV",
    "YUV2RGBA_YUNV",
    "YUV2BGRA_YUNV",

    "YUV2GRAY_UYVY",
    "YUV2GRAY_YUY2",
    //CV_YUV2GRAY_VYUY    = CV_YUV2GRAY_UYVY,
    "YUV2GRAY_Y422",
    "YUV2GRAY_UYNV",
    "YUV2GRAY_YVYU",
    "YUV2GRAY_YUYV",
    "YUV2GRAY_YUNV",

    //! alpha premultiplication
    "RGBA2mRGBA",
    "mRGBA2RGBA",

    //! RGB to YUV 4:2:0 family
    "RGB2YUV_I420",
    "BGR2YUV_I420",
    "RGB2YUV_IYUV",
    "BGR2YUV_IYUV",

    "RGBA2YUV_I420",
    "BGRA2YUV_I420",
    "RGBA2YUV_IYUV",
    "BGRA2YUV_IYUV",
    "RGB2YUV_YV12",
    "BGR2YUV_YV12",
    "RGBA2YUV_YV12",
    "BGRA2YUV_YV12",

    //! Demosaicin
    "BayerBG2BGR",
    "BayerGB2BGR",
    "BayerRG2BGR",
    "BayerGR2BGR",

    "BayerBG2RGB",
    "BayerGB2RGB",
    "BayerRG2RGB",
    "BayerGR2RGB",

    "BayerBG2GRAY",
    "BayerGB2GRAY",
    "BayerRG2GRAY",
    "BayerGR2GRAY",

    //! Demosaicing using Variable Number of Gradients
    "BayerBG2BGR_VNG",
    "BayerGB2BGR_VNG",
    "BayerRG2BGR_VNG",
    "BayerGR2BGR_VNG",

    "BayerBG2RGB_VNG",
    "BayerGB2RGB_VNG",
    "BayerRG2RGB_VNG",
    "BayerGR2RGB_VNG",

    //! Edge-Aware Demosaicing
    "BayerBG2BGR_EA",
    "BayerGB2BGR_EA",
    "BayerRG2BGR_EA",
    "BayerGR2BGR_EA",

    "BayerBG2RGB_EA",
    "BayerGB2RGB_EA",
    "BayerRG2RGB_EA",
    "BayerGR2RGB_EA",

    //! Demosaicing with alpha channel
    "BayerBG2BGRA",
    "BayerGB2BGRA",
    "BayerRG2BGRA",
    "BayerGR2BGRA",

    "BayerBG2RGBA",
    "BayerGB2RGBA",
    "BayerRG2RGBA",
    "BayerGR2RGBA"
};
} // End Namespace DSPatch::DSPatchables
#endif //CONVERTCOLOR_COLOR_TYPES_HPP_
