void kernel simple(global int* output) {
    const uint countX = get_global_id(0);
    output[countX] = countX;
	// Uncomment the following line to check if external definitions can be read
	// output[countX] = MAX_WG_SIZE;
}
