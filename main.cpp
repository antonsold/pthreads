#include <algorithm>
#include <cassert>
#include <chrono>
#include <ctime>
#include <iostream>
#include <pthread.h>
#include <random>

struct Arguments{
    double* arr;
    int arr_length;
    int thread_id;
    int total_threads;
};

struct MergeArgs{
    double* arr;
    int low, mid, high;
};

void merge(double* arr, int low, int mid, int high) {
    int left_size = mid - low + 1, right_size = high - mid;
    auto left = new double[left_size];
    auto right = new double[right_size];

    for (int i = 0; i < left_size; ++i)
        left[i] = arr[i + low];

    for (int j = 0; j < right_size; ++j)
        right[j] = arr[j + mid + 1];

    int i = 0, j = 0, k = low;

    while(i < left_size && j < right_size) {
        if (left[i] <= right[j])
            arr[k++] = left[i++];
        else
            arr[k++] = right[j++];
    }

    while(i < left_size)
        arr[k++] = left[i++];

    while(j < right_size)
        arr[k++] = right[j++];
}

void merge_sort(double* arr, int low, int high) {
    int mid = low + (high - low) / 2;
    if (low < high) {
        merge_sort(arr, low, mid);
        merge_sort(arr, mid + 1, high);
        merge(arr, low, mid, high);
    }
}

void* final_merge(void* args){
    MergeArgs* arg = (MergeArgs*) args;
    merge(arg->arr, arg->low, arg->mid, arg->high);
}

void* merge_sort(void* args) {
    Arguments* arg = (Arguments*) args;
    int q = arg->arr_length / arg->total_threads;
    int low = arg->thread_id * q;
    int high = (arg->thread_id < arg->total_threads - 1 ? (arg->thread_id + 1) * q : arg->arr_length) - 1;
    int mid = low + (high - low) / 2;
    if (low < high) {
        merge_sort(arg->arr, low, mid);
        merge_sort(arg->arr, mid + 1, high);
        merge(arg->arr, low, mid, high);
    }
}

bool compare_arrays(const double* arr1, const double *arr2, int size) {
    for (int i = 0; i < size; ++i) {
        if (arr1[i] != arr2[i])
            return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    const int NUM_THREADS = atoi(argv[1]);
    const int SIZE = atoi(argv[2]);
    const int BLOCK_SIZE = SIZE / NUM_THREADS;

    auto array = new double[SIZE];
    auto sorted_array = new double[SIZE];

    std::uniform_real_distribution<double> unif(-500, 500);
    std::default_random_engine re;
    re.seed(unsigned(time(nullptr)));

    std::cout << std::endl;
    auto threads = new pthread_t[NUM_THREADS];
    auto args = new Arguments[NUM_THREADS];
    auto merge_args = new MergeArgs[NUM_THREADS];

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_THREADS; ++i) {
        args[i] = {array, SIZE, i, NUM_THREADS};
        pthread_create(&threads[i], nullptr, merge_sort, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }
    int num_blocks = NUM_THREADS;

    while(num_blocks > 1) {
        int mult = NUM_THREADS / num_blocks;
        for (int i = 0; i < num_blocks / 2; ++i) {
            int j = i * 2 * mult;
            int high = (j + 2 * mult >= NUM_THREADS) ? SIZE - 1 : BLOCK_SIZE * (j + 2 * mult) - 1;
            merge_args[i] = {array, j * BLOCK_SIZE, BLOCK_SIZE * (j + mult) - 1, high};
            pthread_create(&threads[i], nullptr, final_merge, &merge_args[i]);
        }
        for (int i = 0; i < num_blocks / 2; ++i) {
            pthread_join(threads[i], nullptr);
        }
        num_blocks /= 2;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed1 = end - start;
    std::cout << "Multi-thread sort: " << elapsed1.count() << std::endl;

    start = std::chrono::high_resolution_clock::now();
    std::sort(sorted_array, sorted_array + SIZE);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed2 = end - start;
    std::cout << "Standard sort: " << elapsed2.count() << std::endl;

    assert(compare_arrays(array, sorted_array, SIZE));

    delete[] array;
    delete[] threads;
    delete[] args;
    delete[] merge_args;
    return 0;
}