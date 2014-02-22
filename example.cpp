#include <iostream>
#include <vector>
#include <memory>

#include "boost/bind.hpp"

#include "ThreadPool.h"

using namespace std;

void dotProd(const vector<int> & a, const vector<int> & b, int * c)
{
    for (int i = 0; i < a.size(); ++i) {
        *c += a[i] * b[i];
    }
}

void print(int p)
{
    cout << p << endl;
}

int main(int argc, char const *argv[])
{

    // Pool

    vector<int> a(1000);
    vector<int> b(1000);

    vector<int> results(10000);

    unique_ptr<ThreadPool> tp(new ThreadPool(2));

    for (int i = 0; i < 1000; ++i) {
        a[i] = i * i;
        b[i] = i;
    }

    for (int i = 0; i < 10000; ++i) {
        results[i] = 0;
    }

    int lastJid = 0;

    for (int i = 0; i < 10000; ++i) {
        lastJid = tp->addJob(boost::bind(dotProd, a, b, &results[i]));
    }

    tp->wait(lastJid);

    cout << results[300] << endl;

    // Map

    vector<int> is;

    for (int i = 0; i < 10; ++i) {
        is.push_back(i);
    }

    tp->map(print, is);

    return 0;
}