
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
	
    for (p = 0; p < count -1; p = p + 1;) {
        s = p;
        for (i = p+1; i < count; i = i + 1;) {
            if (numbers[i] < numbers[s]) {
                s = i;
            }
        }
        temp = numbers[p];
        numbers[p] = numbers[s];
        numbers[s] = temp;
    }
    printlines(2);
	for (i = 0; i < count; i = i + 1;) {
		print(numbers[i]);
	}
	printlines(1);

