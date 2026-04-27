#include "PCFG.h"
#include <chrono>
#include <fstream>
#include <sstream>
#include "md5.h"
#include <iomanip>
using namespace std;
using namespace chrono;

// 编译指令如下
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main -O1
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main -O2

int main()
{

    // 下面代码用于测试MD5哈希的正确性
    cout << "Testing MD5Hash correctness..." << endl;
    string test_pws[8] = { "123456", "password", "12345678", "qwerty", "123456789", "12345", "1234", "111111" };
    string test_hashes[8] = {
        "e10adc3949ba59abbe56e057f20f883e",
        "5f4dcc3b5aa765d61d8327deb882cf99",
        "25d55ad283aa400af464c76d713c07ad",
        "d8578edf8458ce06fbc5bb76a58c5ca4",
        "25f9e794323b453885f5181f1b624d0b",
        "827ccb0eea8a706c4c34a16891f84e7b",
        "81dc9bdb52d04dc20036dbd8313ed055",
        "96e79218965eb72c92a549dd5a330112"
    };
    for (int i = 0; i < 8; i++) {
        bit32 state[4];
        MD5Hash(test_pws[i], state);
        stringstream ss;
        for (int i1 = 0; i1 < 4; i1 += 1) {
            ss << std::setw(8) << std::setfill('0') << hex << state[i1];
        }
        if (ss.str() != test_hashes[i]) {
            cout << "MD5Hash test failed for " << test_pws[i] << "!" << endl;
            cout << "Expected: " << test_hashes[i] << "\nGot:      " << ss.str() << endl;
            return 1;
        }
    }
    cout << "MD5Hash test passed!" << endl;//请不要修改这一行

    double time_hash = 0;  // 用于MD5哈希的时间
    double time_guess = 0;  // 哈希和猜测的总时长
    double time_train = 0;  // 模型训练的总时长
    PriorityQueue q;
    auto start_train = system_clock::now();
    q.m.train("/guessdata/Rockyou-singleLined-full.txt");
    q.m.order();
    auto end_train = system_clock::now();
    auto duration_train = duration_cast<microseconds>(end_train - start_train);
    time_train = double(duration_train.count()) * microseconds::period::num / microseconds::period::den;

    q.init();
    cout << "here" << endl;
    int curr_num = 0;
    auto start = system_clock::now();
    // 由于需要定期清空内存，我们在这里记录已生成的猜测总数
    int history = 0;
    // std::ofstream a("./files/results.txt");
   // 全局缓冲区
    static uint32_t hash_dummy_buffer[1000000 * 4];
    int dummy_offset = 0;

    while (!q.priority.empty())
    {
        q.PopNext();
        q.total_guesses = q.guesses.size();
        if (q.total_guesses - curr_num >= 100000)
        {
            cout << "Guesses generated: " << history + q.total_guesses << endl;
            curr_num = q.total_guesses;
            // 在此处更改实验生成的猜测上限
            int generate_n = 10000000;
            if (history + q.total_guesses > 10000000)
            {
                auto end = system_clock::now();
                auto duration = duration_cast<microseconds>(end - start);
                time_guess = double(duration.count()) * microseconds::period::num / microseconds::period::den;
                cout << "Guess time:" << time_guess - time_hash << "seconds" << endl;
                cout << "Hash time:" << time_hash << "seconds" << endl;
                cout << "Train time:" << time_train << "seconds" << endl;
                break;
            }
        }

        // 为了避免内存超限，我们在q.guesses中口令达到一定数目时，将其中的所有口令取出并且进行哈希
        // 然后，q.guesses将会被清空。为了有效记录已经生成的口令总数，维护一个history变量来进行记录
        if (curr_num > 1000000)
        {
            vector<string> batch;
            alignas(16) uint32_t results[16]; // 4 个口令的结果：A0..A3, B0..B3, C0..C3, D0..D3
            auto start_hash = system_clock::now();

            for (const string& pw : q.guesses)
            {
                // 长度 <= 55 字节的口令走 NEON 加速
                if (pw.length() <= 55) {
                    batch.push_back(pw);
                    if (batch.size() == 4) {
                        MD5Hash_NEON_4(batch, results);
                        // 将结果写入全局缓冲区
                        memcpy(hash_dummy_buffer + dummy_offset, results, 16 * sizeof(uint32_t));
                        dummy_offset = (dummy_offset + 16) % (sizeof(hash_dummy_buffer) / sizeof(uint32_t));
                        batch.clear();
                    }
                }
                else {
                    bit32 single_state[4];
                    MD5Hash(pw, single_state);
                    memcpy(hash_dummy_buffer + dummy_offset, single_state, 4 * sizeof(bit32));
                    dummy_offset = (dummy_offset + 4) % (sizeof(hash_dummy_buffer) / sizeof(uint32_t));
                }
            }
            // 处理尾部不足 4 个的口令
            if (!batch.empty()) {
                for (const string& remain_pw : batch) {
                    bit32 single_state[4];
                    MD5Hash(remain_pw, single_state);
                    memcpy(hash_dummy_buffer + dummy_offset, single_state, 4 * sizeof(bit32));
                    dummy_offset = (dummy_offset + 4) % (sizeof(hash_dummy_buffer) / sizeof(uint32_t));
                }
                batch.clear();
            }
            // 以下注释部分用于输出猜测和哈希，但是由于自动测试系统不太能写文件，所以这里你可以改成cout
            // a<<pw<<"\t";
            // for (int i1 = 0; i1 < 4; i1 += 1)
            // {
            //     a << std::setw(8) << std::setfill('0') << hex << state[i1];
            // }
            // a << endl;
            // 在这里对哈希所需的总时长进行计算
            auto end_hash = system_clock::now();
            auto duration = duration_cast<microseconds>(end_hash - start_hash);
            time_hash += double(duration.count()) * microseconds::period::num / microseconds::period::den;
            // 记录已经生成的口令总数
            history += curr_num;
            curr_num = 0;
            q.guesses.clear();
        }
    }
}





// #include "PCFG.h"
// #include <chrono>
// #include <fstream>
// #include <sstream>
// #include "md5.h"
// #include <iomanip>
// using namespace std;
// using namespace chrono;

// // 编译指令如下
// // g++ main.cpp train.cpp guessing.cpp md5.cpp -o main
// // g++ main.cpp train.cpp guessing.cpp md5.cpp -o main -O1
// // g++ main.cpp train.cpp guessing.cpp md5.cpp -o main -O2

// int main()
// {
//     //下面代码用于测试MD5哈希的正确性
//     cout << "Testing MD5Hash correctness..." << endl;
//     string test_pws[8] = {"123456", "password", "12345678", "qwerty", "123456789", "12345", "1234", "111111"};
//     string test_hashes[8] = {
//         "e10adc3949ba59abbe56e057f20f883e",
//         "5f4dcc3b5aa765d61d8327deb882cf99",
//         "25d55ad283aa400af464c76d713c07ad",
//         "d8578edf8458ce06fbc5bb76a58c5ca4",
//         "25f9e794323b453885f5181f1b624d0b",
//         "827ccb0eea8a706c4c34a16891f84e7b",
//         "81dc9bdb52d04dc20036dbd8313ed055",
//         "96e79218965eb72c92a549dd5a330112"
//     };
//     for (int i = 0; i < 8; i++) {
//         bit32 state[4];
//         MD5Hash(test_pws[i], state);
//         stringstream ss;
//         for (int i1 = 0; i1 < 4; i1 += 1) {
//             ss << std::setw(8) << std::setfill('0') << hex << state[i1];
//         }
//         if (ss.str() != test_hashes[i]) {
//             cout << "MD5Hash test failed for " << test_pws[i] << "!" << endl;
//             cout << "Expected: " << test_hashes[i] << "\nGot:      " << ss.str() << endl;
//             return 1;
//         }
//     }
//     cout << "MD5Hash test passed!" << endl; //请不要修改这一行

//     double time_hash = 0;  // 用于MD5哈希的时间
//     double time_guess = 0; // 哈希和猜测的总时长
//     double time_train = 0; // 模型训练的总时长
//     PriorityQueue q;
//     auto start_train = system_clock::now();
//     q.m.train("/guessdata/Rockyou-singleLined-full.txt");
//     q.m.order();
//     auto end_train = system_clock::now();
//     auto duration_train = duration_cast<microseconds>(end_train - start_train);
//     time_train = double(duration_train.count()) * microseconds::period::num / microseconds::period::den;

//     q.init();
//     cout << "here" << endl;
//     int curr_num = 0;
//     auto start = system_clock::now();
//     // 由于需要定期清空内存，我们在这里记录已生成的猜测总数
//     int history = 0;
//     // std::ofstream a("./files/results.txt");
//     while (!q.priority.empty())
//     {
//         q.PopNext();
//         q.total_guesses = q.guesses.size();
//         if (q.total_guesses - curr_num >= 100000)
//         {
//             cout << "Guesses generated: " <<history + q.total_guesses << endl;
//             curr_num = q.total_guesses;

//             // 在此处更改实验生成的猜测上限
//             int generate_n=10000000;
//             if (history + q.total_guesses > 10000000)
//             {
//                 auto end = system_clock::now();
//                 auto duration = duration_cast<microseconds>(end - start);
//                 time_guess = double(duration.count()) * microseconds::period::num / microseconds::period::den;
//                 cout << "Guess time:" << time_guess - time_hash << "seconds"<< endl;//请不要修改这一行
//                 cout << "Hash time:" << time_hash << "seconds"<<endl;//请不要修改这一行
//                 cout << "Train time:" << time_train <<"seconds"<<endl;//请不要修改这一行
//                 break;
//             }
//         }
//         // 为了避免内存超限，我们在q.guesses中口令达到一定数目时，将其中的所有口令取出并且进行哈希
//         // 然后，q.guesses将会被清空。为了有效记录已经生成的口令总数，维护一个history变量来进行记录
//         if (curr_num > 1000000)
//         {
//             auto start_hash = system_clock::now();
//             bit32 state[4];
//             for (string pw : q.guesses)
//             {
//                 // TODO：对于SIMD实验，将这里替换成你的SIMD MD5函数
//                 MD5Hash(pw, state);

//                 // 以下注释部分用于输出猜测和哈希，但是由于自动测试系统不太能写文件，所以这里你可以改成cout
//                 // a<<pw<<"\t";
//                 // for (int i1 = 0; i1 < 4; i1 += 1)
//                 // {
//                 //     a << std::setw(8) << std::setfill('0') << hex << state[i1];
//                 // }
//                 // a << endl;
//             }

//             // 在这里对哈希所需的总时长进行计算
//             auto end_hash = system_clock::now();
//             auto duration = duration_cast<microseconds>(end_hash - start_hash);
//             time_hash += double(duration.count()) * microseconds::period::num / microseconds::period::den;

//             // 记录已经生成的口令总数
//             history += curr_num;
//             curr_num = 0;
//             q.guesses.clear();
//         }
//     }
// }




