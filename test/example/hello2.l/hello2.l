import stdio;
int main ()
{
    int[1] a;
    a[0] = 0;
    f(a);
    return 0;

}
void f(int[] x)
{
    int save = *x;
    int i = 0;
    x = &i;
    printf("%d;%d;%d\n",save,x[0],*x);
    
}