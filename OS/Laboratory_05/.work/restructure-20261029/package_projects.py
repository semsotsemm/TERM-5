from pathlib import Path
import shutil,uuid,hashlib,json,re
from xml.sax.saxutils import escape

W=Path(__file__).resolve().parent;R=W.parents[1];OUT=R/'Laboratory_05'
BACK=W/'original-code';BACK.mkdir(exist_ok=True)
for name in ['native','simulation']:
 if not (BACK/name).exists():shutil.copytree(R/name,BACK/name)
names=['Lab-05a','Lab-05x','Lab-05b','Lab-05c','Lab-05SJF','Lab-05RR','Lab-05CP','Lab-05MQ']
base=(BACK/'native/native.hpp').read_text(encoding='utf8')
base=base[:base.index('int main(int argc, char** argv)')]
base=base.replace('static void check(', '[[maybe_unused]] static void check(')
base=re.sub(r'^static (?=(?:void|int|U64|std::string|const char\*)\s)', '[[maybe_unused]] static ',base,flags=re.M)
comment='// Общие операции ОС: идентификаторы, приоритеты, маски и рабочая нагрузка.\n'
base=base.replace('#pragma once','#pragma once\n'+comment,1)
base=base.replace('return value.substr(0, value.find_last_of(\'/\')) + "/Lab-05x";', 'const bool suffix = value.size() >= 4 && value.compare(value.size() - 4, 4, ".out") == 0;\n    return value.substr(0, value.find_last_of(\'/\')) + "/Lab-05x" + (suffix ? ".out" : "");')
common_mains={
'Lab-05a':'return info();',
'Lab-05x':'return child_main(argc, argv);',
'Lab-05b':'''if (argc < 4 || argc > 6) throw std::invalid_argument("Usage: Lab-05b mask priority1 priority2 [iterations [sleep_ms]]");
        U64 n = argc > 4 ? number(argv[4]) : 1000000;
        int ms = argc > 5 ? signed_number(argv[5]) : 200;
        if (!n || n > 100000000 || ms < 0 || ms > 1000) throw std::invalid_argument("Invalid iterations/sleep_ms");
        return processes(number(argv[1]), signed_number(argv[2]), signed_number(argv[3]), n, ms);''',
'Lab-05c':None}
for osname in ['windows','linux']:
 root=OUT/osname;root.mkdir(parents=True,exist_ok=True)
 (root/'common').mkdir(exist_ok=True)
 (root/'common/Lab05.hpp').write_text(base,encoding='utf8')
 (root/'common/Scheduler.hpp').write_text('#pragma once\n#include <string>\n\nnamespace lab05 {\n    // Общая точка запуска моделей SJF, RR, CP и MQ.\n    int run_scheduler(int argc, char** argv, const std::string& algorithm);\n}\n',encoding='utf8')
 model=(W/'scheduler.hpp').read_text(encoding='utf8').replace('#pragma once','#include "Scheduler.hpp"',1).replace('inline int run_scheduler(', 'int run_scheduler(')
 (root/'common/Scheduler.cpp').write_text(model,encoding='utf8')
 shutil.copy2(BACK/'simulation/tasks.csv',root/'tasks.csv')
 # Сохранённые результаты прежней модели пригодны для сверки отчёта.
 if not (root/'reference-results').exists():shutil.copytree(BACK/'simulation/results',root/'reference-results')
 for name in names:
  project=root/name;project.mkdir(exist_ok=True)
  if name in common_mains:
   main=common_mains[name]
   if name=='Lab-05c':
    if osname=='windows':
     main='''if (argc < 5 || argc > 7) throw std::invalid_argument("Usage: Lab-05c mask process_class priority1 priority2 [iterations [sleep_ms]]");
        U64 n = argc > 5 ? number(argv[5]) : 1000000;
        int ms = argc > 6 ? signed_number(argv[6]) : 200;
        if (!n || n > 100000000 || ms < 0 || ms > 1000) throw std::invalid_argument("Invalid iterations/sleep_ms");
        return threads(number(argv[1]), signed_number(argv[2]), signed_number(argv[3]), signed_number(argv[4]), n, ms);'''
    else:
     main='''if (argc < 4 || argc > 6) throw std::invalid_argument("Usage: Lab-05c mask nice1 nice2 [iterations [sleep_ms]]");
        U64 n = argc > 4 ? number(argv[4]) : 1000000;
        int ms = argc > 5 ? signed_number(argv[5]) : 200;
        if (!n || n > 100000000 || ms < 0 || ms > 1000) throw std::invalid_argument("Invalid iterations/sleep_ms");
        return threads(number(argv[1]), 0, signed_number(argv[2]), signed_number(argv[3]), n, ms);'''
   args='int argc, char** argv' if name!='Lab-05a' else ''
   code=f'''#include "../common/Lab05.hpp"

// { {'Lab-05a':'Вывод параметров текущего процесса и потока.','Lab-05x':'Циклическая нагрузка; используется также дочерними процессами.','Lab-05b':'Запуск двух процессов с заданными маской и приоритетами.','Lab-05c':'Запуск двух потоков и фиксация прогресса при первом завершении.'}[name] }
int main({args}) {{
    try {{
        std::cout << std::unitbuf;
        {main}
    }} catch (const std::exception& error) {{
        std::cerr << "ERROR: " << error.what() << std::endl;
        return 1;
    }}
}}
'''
   header='Lab05.hpp'
  else:
   alg=name.removeprefix('Lab-05')
   code=f'''#include "../common/Scheduler.hpp"

// Моделирование {alg}: реальные пользовательские задания не запускаются.
// Входные параметры и журнал результатов обрабатываются общей моделью.
int main(int argc, char** argv) {{
    return lab05::run_scheduler(argc, argv, "{alg}");
}}
'''
   header='Scheduler.hpp'
  (project/(name+'.cpp')).write_text(code,encoding='utf8')
  guid='{'+str(uuid.uuid5(uuid.NAMESPACE_URL,'os-lab05/'+osname+'/'+name)).upper()+'}'
  linux=osname=='linux'
  globals_extra='''<Keyword>Linux</Keyword><ApplicationType>Linux</ApplicationType><ApplicationTypeRevision>1.0</ApplicationTypeRevision><TargetLinuxPlatform>Generic</TargetLinuxPlatform><LinuxProjectType>{D51BCBC9-82E9-4017-911E-C93873C4EA2B}</LinuxProjectType>''' if linux else '<Keyword>Win32Proj</Keyword><WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>'
  toolset='Remote_GCC_1_0' if linux else 'v145'
  xml=f'''<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64"><Configuration>Debug</Configuration><Platform>x64</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64"><Configuration>Release</Configuration><Platform>x64</Platform></ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals"><ProjectGuid>{guid}</ProjectGuid><RootNamespace>{name.replace('-','_')}</RootNamespace>{globals_extra}</PropertyGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />
  <PropertyGroup Label="Configuration"><ConfigurationType>Application</ConfigurationType><PlatformToolset>{toolset}</PlatformToolset><CharacterSet>Unicode</CharacterSet></PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)'=='Debug'" Label="Configuration"><UseDebugLibraries>true</UseDebugLibraries></PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)'=='Release'" Label="Configuration"><UseDebugLibraries>false</UseDebugLibraries></PropertyGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />
  <PropertyGroup><OutDir>$(SolutionDir)bin\\$(Platform)\\$(Configuration)\\</OutDir><IntDir>$(SolutionDir)obj\\$(ProjectName)\\$(Platform)\\$(Configuration)\\</IntDir><TargetName>{name}</TargetName>{'<RemoteRootDir>~/projects/OS-Lab05</RemoteRootDir><RemoteProjectDir>$(RemoteRootDir)/$(ProjectName)</RemoteProjectDir><RemoteOutDir>$(RemoteRootDir)/bin/$(Platform)/$(Configuration)/</RemoteOutDir><RemoteIntDir>$(RemoteRootDir)/obj/$(ProjectName)/$(Platform)/$(Configuration)/</RemoteIntDir><AdditionalSourcesToCopyMapping>$(ProjectDir)..\\tasks.csv:=$(RemoteOutDir)/tasks.csv;$(ProjectDir)..\\common\\Lab05.hpp:=$(RemoteRootDir)/common/Lab05.hpp;$(ProjectDir)..\\common\\Scheduler.hpp:=$(RemoteRootDir)/common/Scheduler.hpp</AdditionalSourcesToCopyMapping>' if linux else ''}</PropertyGroup>
  <ItemDefinitionGroup>
    <ClCompile><WarningLevel>{'Level3' if linux else 'Level4'}</WarningLevel><LanguageStandard>{'stdcpp17' if not linux else 'stdcpp17'}</LanguageStandard><PrecompiledHeader>NotUsing</PrecompiledHeader>{'<AdditionalOptions>-std=c++17 -pthread %(AdditionalOptions)</AdditionalOptions>' if linux else '<AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions><SDLCheck>true</SDLCheck><ExceptionHandling>Sync</ExceptionHandling><DisableSpecificWarnings>4505;%(DisableSpecificWarnings)</DisableSpecificWarnings>'}</ClCompile>
    <Link>{'<AdditionalOptions>-pthread -lrt %(AdditionalOptions)</AdditionalOptions>' if linux else '<SubSystem>Console</SubSystem>'}</Link>
    {'' if linux else '<PostBuildEvent><Command>copy /Y &quot;$(ProjectDir)..\\tasks.csv&quot; &quot;$(OutDir)tasks.csv&quot;</Command></PostBuildEvent>'}
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)'=='Release'"><ClCompile><Optimization>MaxSpeed</Optimization></ClCompile></ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)'=='Debug'"><ClCompile><Optimization>Disabled</Optimization></ClCompile></ItemDefinitionGroup>
  <ItemGroup><ClCompile Include="{name}.cpp" />{'' if name in common_mains else '<ClCompile Include="..\\common\\Scheduler.cpp" />'}<ClInclude Include="..\\common\\{header}" /></ItemGroup>
  <ItemGroup><None Include="..\\tasks.csv"><CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory></None></ItemGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />
</Project>
'''
  (project/(name+'.vcxproj')).write_text(xml,encoding='utf8')
  if name=='Lab-05b':
   # Lab-05b needs the worker beside its executable even when built alone.
   text=(project/(name+'.vcxproj')).read_text(encoding='utf8')
   text=text.replace('  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />',f'  <ItemGroup><ProjectReference Include="..\\Lab-05x\\Lab-05x.vcxproj"><Project>{{{str(uuid.uuid5(uuid.NAMESPACE_URL,"os-lab05/"+osname+"/Lab-05x")).upper()}}}</Project><LinkLibraryDependencies>false</LinkLibraryDependencies></ProjectReference></ItemGroup>\n  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />')
   (project/(name+'.vcxproj')).write_text(text,encoding='utf8')
  # Fixed debugger defaults are short; actual affinity is supplied in the instructions.
  debugargs={'Lab-05x':'10000 1','Lab-05b':('1 32 32 10000 1' if not linux else '1 0 0 10000 1'),'Lab-05c':('1 32 0 0 10000 1' if not linux else '1 0 0 10000 1'),'Lab-05RR':'--quantum 20','Lab-05CP':'--quantum 20','Lab-05MQ':'--summary'}.get(name,'')
  if not linux:
   (project/(name+'.vcxproj.user')).write_text(f'''<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="Current" xmlns="http://schemas.microsoft.com/developer/msbuild/2003"><PropertyGroup><LocalDebuggerCommandArguments>{debugargs}</LocalDebuggerCommandArguments><LocalDebuggerWorkingDirectory>$(OutDir)</LocalDebuggerWorkingDirectory><DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor></PropertyGroup></Project>
''',encoding='utf8')
 (root/'Laboratory_05.slnx').write_text('<Solution>\n  <Configurations><Platform Name="x64" /></Configurations>\n'+''.join(f'  <Project Path="{n}/{n}.vcxproj" />\n' for n in names)+'</Solution>\n',encoding='utf8')
 (root/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.16)
project(Laboratory_05 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
find_package(Threads REQUIRED)
'''+''.join(f'add_executable({n} {n}/{n}.cpp'+(' common/Scheduler.cpp' if n not in common_mains else '')+f')\ntarget_link_libraries({n} PRIVATE Threads::Threads)\n' for n in names)+'''if(UNIX AND NOT APPLE)
  foreach(target Lab-05a Lab-05x Lab-05b Lab-05c)
    target_link_libraries(${target} PRIVATE rt)
  endforeach()
endif()
if(MSVC)
  foreach(target Lab-05a Lab-05x Lab-05b Lab-05c Lab-05SJF Lab-05RR Lab-05CP Lab-05MQ)
    target_compile_options(${target} PRIVATE /utf-8)
  endforeach()
endif()
configure_file(tasks.csv "${CMAKE_BINARY_DIR}/tasks.csv" COPYONLY)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
configure_file(tasks.csv "${CMAKE_BINARY_DIR}/bin/tasks.csv" COPYONLY)
foreach(config Debug Release RelWithDebInfo MinSizeRel)
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${config}")
  configure_file(tasks.csv "${CMAKE_BINARY_DIR}/bin/${config}/tasks.csv" COPYONLY)
endforeach()
add_dependencies(Lab-05b Lab-05x)
''',encoding='utf8')
 if linux:
  (root/'build.sh').write_text('''#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
mkdir -p bin
for name in Lab-05a Lab-05x Lab-05b Lab-05c Lab-05SJF Lab-05RR Lab-05CP Lab-05MQ; do
    extra=()
    case "$name" in Lab-05SJF|Lab-05RR|Lab-05CP|Lab-05MQ) extra=(common/Scheduler.cpp);; esac
    g++ -std=c++17 -O2 -Wall -Wextra -pthread "$name/$name.cpp" "${extra[@]}" -o "bin/$name" -lrt
done
cp tasks.csv bin/tasks.csv
printf 'Build complete: %s/bin\n' "$PWD"
''',encoding='utf8',newline='\n')
 else:
  (root/'build.cmd').write_text('''@echo off
setlocal
cd /d "%~dp0"
set "LAB_VSWHERE=%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe"
if not exist "%LAB_VSWHERE%" exit /b 1
for /f "usebackq tokens=*" %%i in (`"%LAB_VSWHERE%" -latest -requires Microsoft.Component.MSBuild -property installationPath`) do set "LAB_VS=%%i"
if not defined LAB_VS exit /b 1
"%LAB_VS%\\MSBuild\\Current\\Bin\\MSBuild.exe" Laboratory_05.slnx /m /p:Configuration=Release /p:Platform=x64 /v:minimal
if errorlevel 1 exit /b 1
copy /y tasks.csv bin\\x64\\Release\\tasks.csv >nul
echo Build complete: bin\\x64\\Release
''',encoding='utf8')
print(OUT)
(W/'package.json').write_text(json.dumps({'output':str(OUT),'projects_per_os':names,'backup':str(BACK)},ensure_ascii=False,indent=2),encoding='utf8')
