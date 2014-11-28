#!/usr/bin/python
import os,sys,re

FLT_MAX_POSI='0x1.fffffep127f'
FLT_MIN_NEGA='-0x1.fffffep127f'
FLT_MIN_POSI='0x1.0p-126f'
FLT_MAX_NEGA='-0x1.0p-126f'

paraTypeList={'float':'%e','int':'%d','double':'%lf','uint':'%d','string':'%s'}


def ulpUnit(ulpSize):
  return re.findall(r"([a-zA-Z_]+)",ulpSize)[0]

def ulpNum(ulpSize):
  return re.findall(r"([0-9]+)",ulpSize)[0]

def udebug(ulpSize,returnType):
  #ulpUnit=re.findall(r"([a-zA-Z_]+)",ulpSize)[0]
  #ulpNum=re.findall(r"([0-9]+)",ulpSize)[0]
  text='''
    static const char* INFORNAN;
    static %s ULPSIZE, ULPSIZE_FACTOR;

    const char* env_strict = getenv("OCL_STRICT_CONFORMANCE");

    if (env_strict == NULL || strcmp(env_strict, "0") == 0)
      ULPSIZE_FACTOR = 1000;
    else
      ULPSIZE_FACTOR = 1;
    
    if (isinf(cpu_data[index])){
      INFORNAN="INF";
    }
    else if (isnan(cpu_data[index])){
      INFORNAN="NAN";
    }
    else{
       ULPSIZE=ULPSIZE_FACTOR * cl_%s((cpu_data[index] == 0) ? 1 : cpu_data[index])
               * ((ULPSIZE_FACTOR == 1) ? %s : ( (%s == 0) ? 1 : %s));
    }

#if udebug 
    if (isinf(cpu_data[index])){ 
      if (isinf(gpu_data[index]))
        printf("%s expect:%s\\n", log, INFORNAN);
      else
        printf_c("%s expect:%s\\n", log, INFORNAN);
      }
    else if (isnan(cpu_data[index])){
      if (isnan(gpu_data[index]))
        printf("%s expect:%s\\n", log, INFORNAN);
      else
        printf_c("%s expect:%s\\n", log, INFORNAN);
      }
    else if (diff <= ULPSIZE){
      printf("%s expect:%s\\n", log, ULPSIZE);
      }
    else
      printf_c("%s expect:%s\\n", log, ULPSIZE);
#else
    if (isinf(cpu_data[index])){
      sprintf(log, "%s expect:%s\\n", log, INFORNAN);
      OCL_ASSERTM(isinf(gpu_data[index]),log);
      }
    else if (isnan(cpu_data[index])){
      sprintf(log, "%s expect:%s\\n", log, INFORNAN);
      OCL_ASSERTM(isnan(gpu_data[index]),log);
      }
    else{
      sprintf(log, "%s expect:%s\\n", log, ULPSIZE);
      OCL_ASSERTM(fabs(gpu_data[index]-cpu_data[index]) <= ULPSIZE, log);
      }
#endif
  }
}\n'''%(returnType,\
        ulpUnit(ulpSize),ulpNum(ulpSize),\
        ulpNum(ulpSize), ulpNum(ulpSize),\
        paraTypeList['string'],paraTypeList['string'],\
        paraTypeList['string'],paraTypeList['string'],\
        paraTypeList['string'],paraTypeList['string'],\
        paraTypeList['string'],paraTypeList['string'],\
        paraTypeList['string'],paraTypeList['%s'%(returnType)],\
        paraTypeList['string'],paraTypeList['%s'%(returnType)],\
        paraTypeList['string'],paraTypeList['string'],\
        paraTypeList['string'],paraTypeList['string'],\
        paraTypeList['string'],paraTypeList['%s'%(returnType)])

  return text

def gene2ValuesLoop(values1,values2,inputValues):
  values2=values2+inputValues*len(inputValues)

  for i in inputValues:
    for j in range(0,len(inputValues)):
      values1 += [i]

  return values1,values2

def gene3ValuesLoop(values1,values2,values3,inputValues):
  for i in inputValues:
    for j in range(0,len(inputValues)):
      for k in range(0,len(inputValues)):
        values1 += [i]

  for i in inputValues:
    for j in inputValues:
      for k in range(0,len(inputValues)):
        values2 += [j]

  values3=inputValues*(len(inputValues)**2)
  return values1,values2,values3

class func:
  """ This class will define all needed instance attribute in fundation a c programing file. """

  def __init__(self,name,cpuFuncName,inputType,outputType,values,ulp, cpu_func=''):
    self.funcName = name
    self.cpuFuncName = cpuFuncName
    self.fileName = 'builtin_'+name
    self.inputtype = inputType
    self.outputtype = outputType
    self.values = values
    self.ulp = ulp
    self.cpufunc=cpu_func
    self.cpplines = []
    
#####cpp file required information:
    self.Head='''/*
This file is generated by utest_generator.py.
Usually you need NOT modify this file manually.
But when any bug occured, you can change the value of udebug from 0 to 1,
which can print more values and information to assist debuging the issue.
*/

#include "utest_helper.hpp"
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <string.h>

#define udebug 0
#define FLT_MAX 0x1.fffffep127f
#define FLT_MIN 0x1.0p-126f
#define INT_ULP 0

#define printf_c(...) \\
{\\
  printf("\\033[1m\\033[40;31m");\\
  printf( __VA_ARGS__ );\\
  printf("\\033[0m");\\
}
'''
    #########Execute class itself
    self.geneProcess()

#####Computer vector && argument type:
  def argtype(self,paraN,index):
    return re.findall(r"[a-zA-Z_]+",self.inputtype[paraN][index])[0]

  def argvector(self,paraN,index):
    vector=re.findall(r"[0-9]+",self.inputtype[paraN][index])
    if vector:
      vector=vector[0]
    else:
      vector=1
    return vector

  def returnVector(self,index):
    returnVector=re.findall(r"[0-9]+",self.outputtype[index])
    if returnVector:
      returnVector=returnVector[0]
    else:
      returnVector=1
    return returnVector

  def retType(self,index):
    return re.findall("[a-zA-Z_]+",self.outputtype[index])[0]

  def inputNumFormat(self,paraN,index):
    return paraTypeList['%s'%(self.argtype(paraN,index))]

  def outputNumFormat(self,index):
    return paraTypeList['%s'%(self.retType(index))]

#####Cpu values analyse
  def GenInputValues(self,index):
    #namesuffix=self.inputtype[0][index]
    for i in range(0,self.values.__len__()):
      self.cpplines += [ "const %s input_data%d[] = {%s};" %(self.argtype(i,index),i+1,str(self.values[i]).strip('[]').replace('\'','')) ]
    self.cpplines += [ "const int count_input = sizeof(input_data1) / sizeof(input_data1[0]);" ]
    self.cpplines += [ "const int vector = %s;\n"%(self.argvector(self.inputtype.__len__()-1,index)) ]

#####Cpu Function
  def GenCpuCompilerMath(self,index):
    #namesuffix=self.inputtype[0][index]
    defline='static void cpu_compiler_math(%s *dst, '%(self.retType(index))
    cpufunargs='('
    funcline = ['{']
    vectorargs=[]

    if (self.returnVector(index) == 1 and self.argvector(0,index) != 1):
      for i in range(0,self.values.__len__()):
        defline += 'const %s *src%d'%(self.argtype(i,index),i+1)
        defline += ( i == self.values.__len__()-1 ) and ')' or ','
        vectorargs.append('(')
      for i in range(0,self.values.__len__()):
        for j in range(0,self.vector):
          vectorargs += "x%d%d"%(i+1,j+1)
          vectorargs += ( j == self.vector-1 ) and ');' or ','
          funcline += ["  const %s x%d%d = *(src%d+%d);"%(self.argtype(i,index),i+1,j+1,i+1,j)]

      return 0

    for i in range(0,self.values.__len__()):
      defline += 'const %s *src%d'%(self.argtype(i,index),i+1)
      defline += ( i == self.values.__len__()-1 ) and ')' or ','
      cpufunargs += "x%d"%(i+1)
      cpufunargs += ( i == self.values.__len__()-1 ) and ');' or ','
      funcline += ["  const %s x%d = *src%d;"%(self.argtype(i,index),i+1,i+1)]

    funcline += [ "  dst[0] = %s%s"%(self.cpuFuncName, cpufunargs) ]
    funcline += [ '}'] 

    funcline = [defline] + funcline

    self.cpplines += funcline
#    self.writeCPP( '\n'.join(funcline), 'a', namesuffix)

  def writeCPP(self,content,authority,namesuffix):
    file_object = open("generated/%s_%s.cpp"%(self.fileName,namesuffix),authority)
    file_object.writelines(content)
    file_object.close()

  def writeCL(self,content,authority,namesuffix):
    file_object = open(os.getcwd()+"/../kernels/%s_%s.cl"%(self.fileName,namesuffix),authority)
    file_object.writelines(content)
    file_object.close()

  def nameForCmake(self,content,namesuffix):
    print("generated/%s_%s.cpp"%(self.fileName,namesuffix)),

  def utestFunc(self,index):
    funcLines=[]
    namesuffix=self.inputtype[0][index]
    funcline=[]
    funchead='''
static void %s_%s(void)
{
  int index;
  %s gpu_data[count_input] = {0}, cpu_data[count_input] = {0}, diff=0.0;
  char log[1024] = {0};

  OCL_CREATE_KERNEL(\"%s_%s\");
  OCL_CREATE_BUFFER(buf[0], CL_MEM_READ_WRITE, count_input * sizeof(%s), NULL); 

  globals[0] = count_input;
  locals[0] = 1;
 '''%(self.fileName,namesuffix,\
     self.retType(index),\
     self.fileName, namesuffix,\
     self.retType(index))

    funcline += [funchead]
    for i in range(1,self.values.__len__()+1): 
      funcline += ["  OCL_CREATE_BUFFER(buf[%d], CL_MEM_READ_WRITE, count_input * sizeof(%s), NULL);"%(i,self.argtype(i-1,index))]
      funcline += ["  clEnqueueWriteBuffer( queue, buf[%d], CL_TRUE, 0, count_input * sizeof(%s), input_data%d, 0, NULL, NULL);"%(i,self.argtype(i-1,index),i)]

    funcline += ["  OCL_CREATE_BUFFER(buf[%d], CL_MEM_READ_WRITE, sizeof(int), NULL);"%(self.inputtype.__len__()+1)]
    funcline += ["  clEnqueueWriteBuffer( queue, buf[%d], CL_TRUE, 0, sizeof(int), &vector, 0, NULL, NULL);"%(self.inputtype.__len__()+1)]

	#0=output 1=input1 2=input2 ... len+2=output
    for i in range(0,self.values.__len__()+2): 
      funcline += ["  OCL_SET_ARG(%d, sizeof(cl_mem), &buf[%d]);"%(i,i)]

    funcrun='''
  // Run the kernel:
  OCL_NDRANGE( 1 );
  clEnqueueReadBuffer( queue, buf[0], CL_TRUE, 0, sizeof(%s) * count_input, gpu_data, 0, NULL, NULL);
'''%(self.inputtype.__len__()+1)
    funcline += [ funcrun ]

    funcsprintfa='    sprintf(log, \"'
    funcsprintfb=''
    if (self.returnVector(index) == 1 and self.argvector(0,index) != 1):
      funccompare='''
  for (index = 0; index < count_input/vector; index++)
  {
    cpu_compiler_math( cpu_data + index, '''
    else:
      funccompare='''
  for (index = 0; index < count_input; index++)
  {
    cpu_compiler_math( cpu_data + index,'''

    for i in range(0,self.values.__len__()):
      funccompare += " input_data%d + index"%(i+1)
      funccompare += (self.values.__len__() - 1 == i) and ');' or ','

      funcsprintfa += "input_data%d:"%(i+1)
      funcsprintfa += "%s "%(self.inputNumFormat(i,index))
      funcsprintfb += " input_data%d[index],"%(i+1)

    funcline += [ funccompare ]

    funcsprintfa += " -> gpu:%s  cpu:%s diff:%s\","%(self.outputNumFormat(index),self.outputNumFormat(index),self.outputNumFormat(index))#,self.outputNumFormat(index))
    funcsprintfb += " gpu_data[index], cpu_data[index], diff);"#%(ulpUnit(self.ulp),ulpNum(self.ulp))

    #funcdiff = "    diff = fabs((gpu_data[index]-cpu_data[index])"
    #funcdiff += (self.retType(index) == "int") and ');' or '/(cpu_data[index]>1?cpu_data[index]:1));'
    valuejudge = "    if (std::fpclassify(gpu_data[index]) == FP_SUBNORMAL){ gpu_data[index] = 0; }\n"
    valuejudge += "    if (std::fpclassify(cpu_data[index]) == FP_SUBNORMAL){ cpu_data[index] = 0; }\n"
    funcdiff = "    diff = fabs((gpu_data[index]-cpu_data[index]));"
    funcline += [ valuejudge ]
    funcline += [ funcdiff ]
    funcline += [ funcsprintfa + funcsprintfb ]

    self.cpplines += funcline

    self.cpplines += [ udebug(self.ulp,self.retType(index)) ]
    self.cpplines += [ "MAKE_UTEST_FROM_FUNCTION(%s_%s)"%(self.fileName,namesuffix) ]

  def genCL(self,index):
    namesuffix=self.inputtype[0][index]
    clLine = []
    clhead = '__kernel void %s_%s(__global %s *dst, '%(self.fileName,namesuffix,self.retType(index))
    clvalueDef=''
    clcomputer=''
    tmp=''

    for i in range(0,self.values.__len__()):
      clhead += ' __global %s *src%d,'%(self.argtype(i,index),i+1)
      clvalueDef +=   '  %s x%d = (%s) ('%(self.inputtype[i][index],i+1,self.inputtype[i][index])
      tmp = 'src%d[i * (*vector) + '%(i+1)
      for j in range(0,int(self.argvector(i,index))):
        clvalueDef += tmp + ((int(self.argvector(i-1,index)) == j+1 ) and '%d]);\n'%(j) or '%d],'%(j))
      clcomputer += (self.values.__len__() == i+1) and 'x%d);'%(i+1) or 'x%d,'%(i+1)
      
    clhead += ' __global int *vector) {\n'
    clhead += '  int i = get_global_id(0);'
    clLine += [ clhead ]
    clLine += [ clvalueDef ]
    clLine += [ '  %s ret;'%(self.outputtype[index]) ]
    clLine += [ '  ret = %s('%(self.funcName) + clcomputer ] 

    if (int(self.returnVector(index)) == 1):
      clLine += [ '  dst[i] = ret;' ]
    else:
      for i in range(0,int(self.returnVector(index))):
        clLine += [ '  dst[i * (*vector) + %d] = ret[%d];'%(i,i) ]
    clLine += [ '};' ]

    self.writeCL('\n'.join(clLine),'w',namesuffix)
  
  def geneProcess(self):
    for i in range(0,self.inputtype[0].__len__()):
##########Write Cpp file          
      namesuffix=self.inputtype[0][i]
      self.cpplines = []
      #The head:
      self.cpplines += [self.Head]

      #Parameters:
      self.GenInputValues(i)

      #cpu function generator:
      self.cpplines += [self.cpufunc]

      #Cpu function:
      self.GenCpuCompilerMath(i)

      #utest function
      self.utestFunc(i)

      #kernel cl
      self.genCL(i)

      #CMakelists.txt
      self.nameForCmake(self.fileName,namesuffix)

      self.writeCPP( '\n'.join(self.cpplines) ,'w',namesuffix)
#########End

#def main():
#
#if __name__ == "__main__":
#  main()
