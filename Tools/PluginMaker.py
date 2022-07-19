"""
PluginMaker
Created By: Richard Wardlow

Tool to help create new FlowCV plugins both internal and external types.
Run with --help for usage information.
"""

import sys, os
from optparse import OptionParser


def IoTypesList():
    ioTypesFile = open("../FlowCV_SDK/third-party/dspatch/include/dspatch/ComponentTypes.hpp", 'r')
    Lines = ioTypesFile.readlines()
    ioTypesFile.close()
    ioList = []
    x = 0
    foundList = False
    while x < len(Lines) and not foundList:
        if 'IoType' in Lines[x]:
            x += 2
            foundEnd = False
            index = 0
            while not foundEnd:
                ioTmp = Lines[x].split('_')
                ioTmpSub = ioTmp[2:]
                ioName = "_"
                ioName = ioName.join(ioTmpSub)
                if ',' in ioName:
                    ioName = ioName[:-2]
                else:
                    ioName = ioName[:-1]
                ioEntry = [index, ioName]
                ioList.append(ioEntry)
                x += 1
                index += 1
                if '}' in Lines[x]:
                    foundEnd = True
                    foundList = True
        x += 1
    return ioList


def CategoryList():
    catTypesFile = open("../FlowCV_SDK/third-party/dspatch/include/dspatch/ComponentTypes.hpp", 'r')
    Lines = catTypesFile.readlines()
    catTypesFile.close()
    catList = []
    x = 0
    foundList = False
    while x < len(Lines) and not foundList:
        if 'Category' in Lines[x]:
            x += 2
            foundEnd = False
            index = 0
            while not foundEnd:
                catTmp = Lines[x].split('_')
                catTmpSub = catTmp[1:]
                catName = "_"
                catName = catName.join(catTmpSub)
                if ',' in catName:
                    catName = catName[:-2]
                else:
                    catName = catName[:-1]
                catEntry = [index, catName]
                catList.append(catEntry)
                if '};' in Lines[x + 1]:
                    foundEnd = True
                    foundList = True
                x += 1
                index += 1
        x += 1
    return catList


def Make_Plugin(options):
    if options.inputs is '' and options.outputs is '':
        print('Need to Specify at Least 1 input or 1 output')
        return
    if options.itype is '' and options.otype is '':
        print('Need to Specify I/O Types for inputs and/or outputs')
        return
    ioList = IoTypesList()
    catList = CategoryList()
    print('Making Plugin...')
    sourceFileBaseName = options.name
    pluginClassTmp = options.name.split('_')
    for x in range(0, len(pluginClassTmp)):
        pluginClassTmp[x] = pluginClassTmp[x].capitalize()
    pluginClassName = ""
    pluginClassName = pluginClassName.join(pluginClassTmp)
    componentName = "_"
    componentName = componentName.join(pluginClassTmp)

    outputDirName = ''
    if options.ext == 1:
        outputDirName = 'Plugins'
    else:
        outputDirName = 'Internal_Nodes'

    inputDirName = ''
    if options.ext == 1:
        inputDirName = 'Plugin'
    else:
        inputDirName = 'Internal_Node'

    inputTempBaseName = ''
    if options.ext == 1:
        inputTempBaseName = 'plugin_template'
    else:
        inputTempBaseName = 'internal_template'

    # Create Dir
    if not os.path.exists('../' + outputDirName + '/' + pluginClassName):
        os.makedirs('../' + outputDirName + '/' + pluginClassName)

    # Make CMake File
    if options.ext == 1:
        cmakeTemplateFile = open('../Templates/' + inputDirName + '/CMakeLists.txt', 'r')
        cmakeLines = cmakeTemplateFile.readlines()
        cmakeTemplateFile.close()
        cmakeOutFile = open('../Plugins/' + pluginClassName + '/CMakeLists.txt', 'w')
        for line in cmakeLines:
            if 'project(Plugin_Name)' in line:
                line = 'project(' + pluginClassName + ')\n'
            if 'source_file.cpp' in line:
                line = '        ' + sourceFileBaseName + '.cpp\n'
            cmakeOutFile.write(line)
        cmakeOutFile.close()

    # Make hpp File
    hppTemplateFile = open('../Templates/' + inputDirName + '/' + inputTempBaseName + '.hpp', 'r')
    hppLines = hppTemplateFile.readlines()
    hppTemplateFile.close()
    hppOutFile = ''
    hppOutFile = open('../' + outputDirName + '/' + pluginClassName + '/' + sourceFileBaseName + '.hpp', 'w')

    for line in hppLines:
        if '// Plugin Template' in line:
            line = line.replace('Template', pluginClassName)
        if 'FLOWCV_PLUGIN_TEMPLATE_HPP_' in line:
            line = line.replace('TEMPLATE', options.name.upper())
        if 'PluginName' in line:
            line = line.replace('PluginName', pluginClassName)
        hppOutFile.write(line)
    hppOutFile.close()

    # Add hpp to internal nodes include file
    if options.ext == 0:
        incFile = open('../Internal_Nodes/internal_nodes.hpp', 'r')
        hppLines = incFile.readlines()
        incFile.close()
        incFile = open('../Internal_Nodes/internal_nodes.hpp', 'w')

        writeOnce = True
        for line in hppLines:
            if len(line) <= 1:
                if writeOnce:
                    incFile.write('#include "' + pluginClassName + '/' + sourceFileBaseName + '.hpp"\n')
                    writeOnce = False
            incFile.write(line)
        incFile.close()

    # Make cpp File
    cppTemplateFile = open('../Templates/' + inputDirName + '/' + inputTempBaseName + '.cpp', 'r')
    cppLines = cppTemplateFile.readlines()
    cppTemplateFile.close()
    cppOutFile = open('../' + outputDirName + '/' + pluginClassName + '/' + sourceFileBaseName + '.cpp', 'w')
    inputList = []
    inTypeList = []
    if options.inputs is not '':
        inputList = options.inputs.split(',')
        inTypeList = options.itype.split(',')
    outputList = []
    outTypeList = []
    if options.outputs is not '':
        outputList = options.outputs.split(',')
        outTypeList = options.otype.split(',')
    for line in cppLines:
        if '// Plugin Template' in line:
            line = line.replace('Template', pluginClassName)
        if inputTempBaseName + '.hpp' in line:
            line = line.replace(inputTempBaseName, sourceFileBaseName)
        if 'PluginName' in line:
            line = line.replace('PluginName', pluginClassName, 2)
        if 'SetComponentName_' in line:
            line = line.replace('Plugin_Name', componentName)
        if 'SetComponentCategory_' in line:
            line = line.replace('Other', catList[options.category][1])
        if 'SetComponentAuthor_' in line:
            line = line.replace('"Author"', '"' + options.author + '"')
        if 'SetComponentVersion_' in line:
            line = line.replace('0.0.0', options.version)
        if '// 1 inputs' in line:
            if len(inputList) > 0:
                line = line.replace('1', str(len(inputList)))
            else:
                line = line.replace('1', '0')
        if 'SetInputCount_' in line:
            if len(inputList) > 0:
                newInputs = '    SetInputCount_( ' + str(len(inputList)) + ', '
                inName = '{'
                cnt = 0
                for input in inputList:
                    inName += '"' + input + '"'
                    if len(inTypeList) > 1 and cnt != len(inTypeList) - 1:
                        inName += ", "
                    cnt += 1
                inName += '}, {'
                cnt = 0
                for inType in inTypeList:
                    inName += 'IoType::Io_Type_' + ioList[int(inType)][1]
                    if len(inTypeList) > 1 and cnt != len(inTypeList) - 1:
                        inName += ", "
                    cnt += 1
                inName += '} );'
                line = newInputs + inName + '\n'
            else:
                line = '    SetInputCount_(0);\n'
        if '// 1 outputs' in line:
            if len(outputList) > 0:
                line = line.replace('1', str(len(outputList)))
            else:
                line = line.replace('1', '0')
        if 'SetOutputCount_' in line:
            if len(outputList) > 0:
                newOutputs = '    SetOutputCount_( ' + str(len(outputList)) + ', '
                outName = '{'
                cnt = 0
                for output in outputList:
                    outName += '"' + output + '"'
                    if len(outTypeList) > 1 and cnt != len(outTypeList) - 1:
                        outName += ", "
                    cnt += 1
                outName += '}, {'
                cnt = 0
                for outType in outTypeList:
                    outName += 'IoType::Io_Type_' + ioList[int(outType)][1]
                    if len(outTypeList) > 1 and cnt != len(outTypeList) - 1:
                        outName += ', '
                    cnt += 1
                outName += '} );'
                line = newOutputs + outName + '\n'
            else:
                line = '    SetOutputCount_(0);\n'
        if '// Handle Input Code Here' in line:
            if len(inputList) > 0:
                inputHandler = '    // Input 1 Handler\n'
                inputHandler += '    auto in1 = inputs.GetValue<type>( 0 );\n'
                if ioList[int(inTypeList[0])][1] == 'CvMat':
                    inputHandler = inputHandler.replace('type', 'cv::Mat')
                else:
                    inputHandler = inputHandler.replace('type', ioList[int(inTypeList[0])][1].lower())
                inputHandler += '    if ( !in1 ) {\n'
                inputHandler += '        return;\n'
                inputHandler += '    }\n'
                inputHandler += '\n    // Do Something with Input\n'
                inputHandler += '    // For Example\n'
                inputHandler += '    if (!in1->empty()) {\n'
                inputHandler += '        if (IsEnabled()) {\n'
                inputHandler += '            // Process Image\n\n'
                inputHandler += '        } else {\n'
                inputHandler += '            // Copy Original to Output (pass thru)\n'
                inputHandler += '            in1->copyTo(frame_);\n'
                inputHandler += '        }\n'
                if len(outputList) == 0:
                    inputHandler += '        outputs.SetValue(0, frame_);\n'
                    inputHandler += '    }\n'
                line = inputHandler
            else:
                line = ''
        if '// Handle Output Code Here' in line:
            spaces = ''
            if len(inputList) > 0:
                spaces = '    '
            if len(outputList) > 0:
                outputHandler = '\n' + spaces + '    // Do Something with Output\n'
                outputHandler += spaces + '    outputs.SetValue(0, frame_);\n'
                if len(inputList) > 0:
                    outputHandler += '    }\n'

                line = outputHandler
            else:
                line = ''

        cppOutFile.write(line)

    cppOutFile.close()

    # Add to CMake Project if Option Set
    if options.ext == 1:
        if options.add == 1:
            projFile = open('../CMakeLists.txt', 'r')
            projLines = projFile.readlines()
            projFile.close()
            newProjFile = open('../CMakeLists.txt', 'w')
            foundPluginStart = False
            foundPluginEnd = False
            alreadyAdded = False
            for line in projLines:
                if '# FlowCV Plugin Modules' in line:
                    foundPluginStart = True
                if foundPluginStart:
                    if pluginClassName in line:
                        alreadyAdded = True
                    if len(line) <= 2:
                        foundPluginEnd = True
                if foundPluginStart and foundPluginEnd and not alreadyAdded:
                    line = '    add_subdirectory(./Plugins/' + pluginClassName + ')\n\n'
                    foundPluginStart = False
                    foundPluginEnd = False

                newProjFile.write(line)

            newProjFile.close()

def Main():
    parser = OptionParser()
    parser.add_option(
        '-n', '--name',
        dest='name',
        type='string',
        default=None,
        help='Specify New Plugin Name')
    parser.add_option(
        '-a', '--author',
        dest='author',
        type='string',
        default=None,
        help='Specify The Plugin Author')
    parser.add_option(
        '-v', '--ver',
        dest='version',
        type='string',
        default='0.1.0',
        help='Specify Plugin Version: 0.1.0')
    parser.add_option(
        '-d', '--add',
        dest='add',
        type='int',
        default=1,
        help='Add to Main CMake Project File')
    parser.add_option(
        '-c', '--category',
        dest='category',
        type='int',
        default=None,
        help='Specify Plugin Category: Source, Filter, etc.')
    parser.add_option(
        '-i', '--inputs',
        dest='inputs',
        type='string',
        default='',
        help='Specify Number of Inputs with CSV Names: in,in,in')
    parser.add_option(
        '-o', '--outputs',
        dest='outputs',
        type='string',
        default='',
        help='Specify Number of Outputs with CSV Names: out,color,depth')
    parser.add_option(
        '-p', '--intype',
        dest='itype',
        type='string',
        default='',
        help='Specify Input Type, must match number of inputs (use -l 1 for list): 1,2,4')
    parser.add_option(
        '-t', '--outtype',
        dest='otype',
        type='string',
        default='',
        help='Specify Output Type, must match number of outputs (use -l 1 for list): 1,2,4')
    parser.add_option(
        '-l', '--list',
        dest='list',
        type='int',
        default=None,
        help='Set to 1 to get List of Categories and I/O Types (use -l 1 for list)')
    parser.add_option(
        '-e', '--ext',
        dest='ext',
        type='int',
        default=1,
        help='Create Internal or External Component, default 1, set to 0 for Internal')

    (options, args) = parser.parse_args()

    if options.list == 1:
        print("Categories:")
        catList = CategoryList()
        for cat in catList:
            print('  ', cat[0], ' = ', cat[1])
        print('\n')
        print("I/O Types:")
        ioList = IoTypesList()
        for io in ioList:
            print('  ', io[0], ' = ', io[1])
        return 0
    elif options.name is not None and \
        options.author is not None and \
        options.category is not None:
        Make_Plugin(options)
    else:
        print("Need to Specify All Parameters")
        print("use --help for help")


if __name__ == '__main__':
    Main()
