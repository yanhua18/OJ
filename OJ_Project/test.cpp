#include<stdio.h>
#include"httplib.h"

using namespace httplib;
void func(const Request& req,Response& resp)
{

    resp.set_content("<html>hello bite</html>","123");
}
int main()
{
    Server svr;
    svr.Get("/",func);

    svr.listen("0,0,0,0",19999);
    return 0;
}
