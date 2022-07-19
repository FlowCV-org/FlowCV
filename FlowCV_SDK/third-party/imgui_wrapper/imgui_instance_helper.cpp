//
// Created by Richard on 1/10/2022.
//
#include "imgui_instance_helper.hpp"

std::string CreateControlString(const char *text, const char *unique_name)
{
    std::string idString = std::string(text);
    idString += "##";
    idString += unique_name;

    return idString;
}
