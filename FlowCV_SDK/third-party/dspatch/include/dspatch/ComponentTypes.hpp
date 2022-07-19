//
// DSPatch Common Data Types
//

#ifndef DSPATCH_MANAGERS_COMMON_TYPES_HPP_
#define DSPATCH_MANAGERS_COMMON_TYPES_HPP_

namespace DSPatch
{

enum class IoType
{
    Io_Type_Unspecified,
    Io_Type_CvMat,
    Io_Type_Bool,
    Io_Type_Int,
    Io_Type_Float,
    Io_Type_String,
    Io_Type_JSON,
    Io_Type_Bool_Array,
    Io_Type_Int_Array,
    Io_Type_Float_Array,
    Io_Type_String_Array
};

enum class Category
{
    Category_Source,
    Category_Output,
    Category_Draw,
    Category_Color,
    Category_Filter,
    Category_Merge,
    Category_Transform,
    Category_Views,
    Category_Feature_Detection,
    Category_Object_Detection,
    Category_OpenVino,
    Category_Utility,
    Category_Other,
    Category_Experimental
};

} // End Namespace DSPatch

#endif //DSPATCH_MANAGERS_COMMON_TYPES_HPP_
