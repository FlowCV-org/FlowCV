//
// FlowCV Common Types
//

#include "DSPatch.h"

namespace DSPatch
{
    const std::map<DSPatch::Category, const char *>& getCategories()
    {
        static const auto* category_strings = new std::map<DSPatch::Category, const char *>{
            {Category::Category_Source, "Source"},
            {Category::Category_Output, "Output"},
            {Category::Category_Draw, "Draw"},
            {Category::Category_Color, "Color"},
            {Category::Category_Filter, "Filter"},
            {Category::Category_Merge, "Merge"},
            {Category::Category_Transform, "Transform"},
            {Category::Category_Views, "Views"},
            {Category::Category_Feature_Detection, "Feature Detection"},
            {Category::Category_DNN, "DNN"},
            {Category::Category_Utility, "Utility"},
            {Category::Category_Other, "Other"},
            {Category::Category_Experimental, "Experimental"}
        };

        return *category_strings;
    }
}