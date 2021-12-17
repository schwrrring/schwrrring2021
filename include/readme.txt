By convention, include directory is for header files, but modern practice suggests that include directory must strictly
contain headers that need to be exposed publicly. A thing to note here is the use of another directory inside the include
directory. What is even more interesting is that it has a name same as that of your project. The reason to do this is to
give a sense of specification when someone tries to use your library. Thus to use your library, one has to use the code

#include <Project_Name/public_header.h>