#include <iostream>
#include <unordered_map>
#include <fstream>
#include <cstdint>
#include <string>
using namespace std;

union ben{
    uint16_t num;
    char c[2];
};

void error(bool condition, const string &msg)
{
    if(!condition) {
	cout << msg<<endl;
	exit(1);
    }
}

class Dictionary {
    //able to lookup the has both ways.
    public:
	void insert(uint16_t i, const string &str) {
	    hash_.insert(make_pair(i, str));
	    hash2_.insert(make_pair(str, i));
	}

	bool exist(uint16_t i) {
	    return end(hash_) != hash_.find(i);
	}

	bool exist(const string &str) {
	    return end(hash2_) != hash2_.find(str);
	}

	uint16_t get_num(const string &str){
	    auto found = hash2_.find(str);
	    error(end(hash2_) != found, "get_num error");
	    return found->second;
	}

	string get_str(uint16_t i) {
	    auto found = hash_.find(i);
	    error(end(hash_) != found, "get_str error");
	    return found->second;
	}
	Dictionary() {
	    for(uint16_t i=0; i<256; ++i) {
		char c= i;
		hash_.insert(make_pair(i, string() + c));
		hash2_.insert(make_pair(string() + c, i));
	    }
	}
	void print() {
	    for(auto &p:hash_) {
		if(p.first >= 256)
		    cout<<"["<<p.first<<", "<<p.second<<"]\n";
	    }
	}

    private:
	unordered_map<uint16_t, string> hash_;
	unordered_map<string, uint16_t> hash2_;
};

class MyStream {
    //This class allows user to peek the next int16_t in the file.
    public: 
	bool read(uint16_t &i) {
	    ben bb;
	    if(ifs_.get(bb.c[0]) && ifs_.get(bb.c[1])) {
		i = bb.num;
		return true;
	    } else {
		return false;
	    }
	}
	bool peek(uint16_t &i) {
	    if(read(i)) {
		ben bb;
		bb.num = i;
		ifs_.putback(bb.c[1]);
		ifs_.putback(bb.c[0]);
		return true;
	    } else {
		return false;
	    }
	}
	MyStream(const string &filename) { 
	    ifs_.open(filename, ios::binary);
	}
	~MyStream(){
	    ifs_.close();
	}
    private:
	ifstream ifs_;
};

void uncompress(const string &input, const string &output)
{
    MyStream stream(input);
    ofstream ofs(output, ios::binary);
    Dictionary dict;
    uint16_t current_i;
    int key = 256;
    string current_str;
    string last_inserted_hash_str;
    while(stream.read(current_i)) {
	cout<<"hello world, current i ="<<current_i<<endl;
	bool must_exist = dict.exist(current_i);
	error(must_exist, "cannot decode in uncompress!");
	string sss= dict.get_str(current_i);
	cout <<"sss="<<sss<<endl;
	ofs << sss;
	uint16_t next_i;
	if(stream.peek(next_i)) {
	    cout <<"peek OK for "<<next_i<<endl;
	    if(dict.exist(next_i)) {
		string next_str = dict.get_str(next_i);
		dict.insert(key++, sss+ next_str[0]);
	    } else {
		error(key == next_i, "help me.. i am confused!");
		dict.insert(key++, sss+ sss[0]);
	    }
	} 
    }
    dict.print();
    ofs.close();
}

void compress(const string &input, const string &output)
{
    ifstream ifs(input);
    ofstream ofs(output, ios::binary);
    string current_str;
    Dictionary dict;
    int16_t key = 256;
    char c;
    while(ifs.get(c)){
	current_str += c;
	int pk = ifs.peek();
	string key_str = current_str + static_cast<char>(pk);
	if(dict.exist(current_str) && 
		(!dict.exist(key_str) || pk==EOF)) {
	    ben bb;
	    bb.num = dict.get_num(current_str);
	    ofs.write(bb.c,2);
	    if(key < 60000) {
		dict.insert(key++, key_str);
	    } else {
		cout <<"reached limit, no hash entry will be made for "<<key_str<<endl;
	    }
	    current_str.clear();
	}
    }
    ifs.close();
}

int main(int argc, char *argv[])
{
    if(argc != 4) {
	cout <<argv[0]<<" [compress/uncompress] input output"<<endl;
    }
    string option(argv[1]);
    string input(argv[2]);
    string output(argv[3]);

    if(0 == option.compare("compress")) {
	compress(input, output);
    } else if(0 == option.compare("uncompress")) {
	uncompress(input, output);
    } else {
	cout <<"invalid option!"<<endl;
    }
}
