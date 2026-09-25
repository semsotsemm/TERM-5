extern "C" {
    int sum(int x, int y) { return x + y; }
    int sub(int x, int y) { return x - y; }
    int mul(int x, int y) { return x * y; }
    int my_div(int x, int y) {
        if (y == 0){
            return 0;
        }
        return x / y;
    }
}