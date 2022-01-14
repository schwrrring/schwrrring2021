#Cmake
## Basic Library import - 
### difference between target_link_libraries and target_include_directories

I assume you are having issues understanding how builds work. I'll try to help you fill in the gaps.

A target is an executable or a library built by cmake.

When creating a library A you need to provide an API (header/include files) and compiled code (.lib).

To use library A in project B, you need to include the header files. You can then compile B. However to create a binary you need to link with the compiled A.

<ins>target_include_directories tells cmake where to find the API header files</ins> so you can include them from B.

<ins>  target_link_directories and target_link_libraries tell cmake where to find the library's compiled code.</ins> If the library is header-only there is no need to link.

## Understanding Inheritance in CMAKE PUBLIC|PRIVATE|Interface:
https://leimao.github.io/blog/CMake-Public-Private-Interface/

## Rules I follow:
###target_include_directories
  * use Interface and Public only for the include Folder of a library
  * use Private von src folder

## Juce-Cmake
https://forum.juce.com/t/native-built-in-cmake-support-in-juce/38700
=> hier gibt es eine erwähnung von examples im repository von juce und vom readme


