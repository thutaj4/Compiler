
    int numbers[30];
    int count;
    int i;
    int s;
    int p;
    int temp;
    read(count);
    i = 0;
    while (i < count) {
        read(numbers[i]);
	i = i + 1;
    }
	
    printlines(2);
    for (i = 0; i < count; i = i + 1;) {
        print(numbers[i]);
    }
    printlines(1);

