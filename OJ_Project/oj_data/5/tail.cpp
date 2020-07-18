#ifndef CompileOnline
// 这是为了编写用例的时候有语法提示. 实际线上编译的过程中这个操作是不生效的.
#include"header.cpp"
#endif

///////////////////////////////////////////////////////
// 此处约定:
// 1. 每个用例是一个函数
// 2. 每个用例从标准输出输出一行日志
// 3. 如果用例通过, 统一打印 [TestName] ok!
// 4. 如果用例不通过, 统一打印 [TestName] failed! 并且给出合适的提示.
///////////////////////////////////////////////////////

void Test1() {
    int ret = Solution().NumberOf1(3);
    if(ret==2)
    {
      std::cout<<"it is ok!!"<<std::endl;
    }
    else{
      std::cout<<"ERROR!"<<std::endl;
    }
}
int main(){

    Test1();
    return 0;
}
