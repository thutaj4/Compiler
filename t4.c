
    int start;
    int end;
    int inc;
    int current;
    int c;
    int k;

    read(start, end, inc);
    current = start;
    while (current <= end) {
        c = (current - 32) * 5/9;
        k = (current + 460) * 5/9;
        print(current, c, k);
	printlines(1);
        current = current + inc;
    }
}
