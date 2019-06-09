void kernel simple(global int* output) {
    const uint countX = get_global_id(0);
    output[countX] = countX;
}
