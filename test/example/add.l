import stdio;

static int g = 3;
static int c;

int main(int argc, char **argv)
{
    int i;
    int j = 0;

    printf("%d", 1 + 0);
    printf(";%d", 2 + j);
    i = 2;
    j = 1;
    printf(";%d", i + j);
    printf(";%d", j + g);
    printf(";%d", i + g);
    printf(";%d", f(5));     
    printf(";%d", f(5) + 1);
    printf(";%d", f(5) + i);  
    c = 5;
    printf(";%d", f(3) + c);
    i = 9;
    printf(";%d", f(i));
    j = 1;
    printf(";%d", f(i) + j);
    puts("");
    return 0;
}

int f(int i)
{
    return i + 1;
}