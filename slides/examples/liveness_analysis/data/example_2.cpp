int main() {
// in: {}
int a;
int b;
int c;
int d;
int x;
b1: a = 3; 
    b = 5;
    d = 4;
    x = 100; //x is never being used later thus not in the out set {a,b,d}
    if (a > b) {
// out: {a,b,d}    //union of all (in) successors of b1 => b2: {a,b}, and b3:{b,d}  

// in: {a,b}
b2: c = a + b;
    d = 2;
// out: {b,d}

// in: {b,d}
}
b3:
    c = 4;
    return b * d + c;
// out:{}
}
