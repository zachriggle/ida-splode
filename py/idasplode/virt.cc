#include <iostream>
#include <stdio.h>
using namespace std;


struct Base
{
    Base(int i=123) { cout << "Base " << i << endl; }
    virtual ~Base();

    virtual void Fn()=0;
};
struct Mid : public virtual Base
{
    Mid(char c='z') { cout << "Mid " << c << endl; }
    virtual ~Mid(){};

    void Fin() { cout << "Nada" << endl; }
};
struct Low : public Mid
{
    Low()
    : Base(999), Mid('x')
    {
        cout << "Low" << endl;
    }
    ~Low() {}
};


int main(int argc, char** argv) {
    printf("Dafuq\n");
    // Low l;
    cout << "What?" << endl;
}