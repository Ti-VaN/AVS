#include <iostream>
#include <string>
#include <chrono>
#include <fstream>

using namespace std;

void testRAM(long size, long double& timeOfWrite, long double& timeOfRead) {
	long long int length = size / 4;
	int* arr = new int[length];
	int* temp = new int[length];
	for (int i = 0; i < length; i++) {
		temp[i] = rand();
	}
	auto start = chrono::steady_clock::now();
	for (int i = 0; i < length; i++) {
		arr[i] = temp[i];
	}
	auto end = chrono::steady_clock::now();
	chrono::duration<double> elapsed_seconds = end - start;
	timeOfWrite = elapsed_seconds.count();
	delete[] temp;
	char t;
	start = chrono::steady_clock::now();
	for (int i = 0; i < length; i++) {
		t = arr[i];
	}
	end = chrono::steady_clock::now();
	elapsed_seconds = end - start;
	timeOfRead = elapsed_seconds.count();
	delete[] arr;
}

void testMemory(string address, long size, long double& timeOfWrite, long double& timeOfRead) {
	ofstream out(address + "\testMemory.txt");
	int len = size / 2;
	int* s = new int[len];
	for (int i = 0; i < len; i++) {
		s[i] = (rand() * (1.0 / (static_cast<double>(RAND_MAX) + 1.0)) * 10);
	}
	auto start = chrono::steady_clock::now();
	for (int i = 0; i < len; i++) {
		out << s[i] << " ";
	}
	auto end = chrono::steady_clock::now();
	chrono::duration<double> elapsed_seconds = end - start;
	out.close();
	timeOfWrite = elapsed_seconds.count();
	ifstream in(address + "\testMemory.txt");
	string now;
	start = chrono::steady_clock::now();
	for (int i = 0; i < len; i++) {
		in >> now;
	}
	end = chrono::steady_clock::now();
	elapsed_seconds = end - start;
	in.close();
	timeOfRead = elapsed_seconds.count();
	delete s;
}

void printToFile(string type, string sizeType, int count, long size, long double rdis, long double* read,
	long double* write, long double srread, long double srwrite, long double wrdis, int buffer) {
	fstream fileo("result.csv", ios_base::app);
	if (ifstream("result.csv").peek() == EOF) {
		fileo << "MemoryType;" << "BlockSize;" << "ElementType;" << "BufferSize;" << "LaunchNum;" << "Timer;" << "WriteTime;" << "AverageWriteTime;" <<
			"WriteBandwidth;" << "AbsError(write);" << "RelError(write);" << "ReadTime;" << "AverageReadTime;" << "ReadBandwidth;" <<
			"AbsError(read); " << "RelError(read);" << endl;
	}
	else {
		fileo << endl;
	}
	for (int i = 0; i < count; i++) {
		fileo << type << ";" << size << " byte;" << "int" << ";" << buffer << ";" << (i + 1) << ";" << "chrono;" << to_string(write[i]) << ";" << to_string(srwrite) << ";"
			<< to_string(size / write[i] / 1000000) << "Mb/s;" << to_string(sqrt(wrdis)) << ";" << to_string(sqrt(wrdis) / count) << ";" << to_string(read[i]) << ";"
			<< to_string(srread) << ";" << to_string(size / read[i] / 1000000) << "Mb/s;" << to_string(sqrt(rdis)) << ";" << to_string(sqrt(rdis) / count) << ";" << endl;
	}
	fileo.close();
}

int main()
{
	srand(time(NULL));
	setlocale(LC_ALL, "Russian");
	string type, sizeType, address = "";
	int count = 3;
	int userInput;
	long l[] = { 262144, 1048576, 6291456, 7340032 };
	long size = 0;
	string memoryType[] = { "HDD", "flash" };
	long double wrm2 = 0, wrm = 0, wrdis, rm = 0, rm2 = 0, rdis = 0, srwrite = 0, srread = 0;
	int num, numStep;
	cout << "1 - ручной ввод" << endl << "2 - тест оперативной памяти" << endl << "3 - тест из 20 испытаний" << endl;
	cin >> userInput;
	switch (userInput)
	{
	case 1:
		cout << "Пример входных данных: RAM 1024 Kb 10\n";
		cin >> type;
		cin >> size;
		cin >> sizeType;
		cin >> count;
		if (sizeType == "Mb") {
			size *= 1048576;
		}
		else if (sizeType == "Kb") {
			size *= 1024;
		}
		num = 1;
		numStep = 1;
		break;
	case 2:
		type = "RAM";
		num = 4;
		break;
	case 3:
		num = 2;
		count = 1;
		numStep = 20;	
		break;
	}

	long double* write = new long double[count];
	long double* read = new long double[count];

	if (type == "RAM") {
		for (int j = 0; j < num; j++) {
			if (num > 1) {
				size = l[j];
				wrm2 = 0, wrm = 0, wrdis, rm = 0, rm2 = 0, rdis = 0, srwrite = 0, srread = 0;
			}
			for (int i = 0; i < count; i++) {
				testRAM(size, write[i], read[i]);
				wrm += write[i] / count;
				wrm2 += pow(write[i], 2) / count;
				rm += read[i] / count;
				rm2 += pow(read[i], 2) / count;
				srwrite += write[i];
				srread += read[i];
			}
			srwrite /= count;
			srread /= count;
			wrdis = abs(pow(wrm, 2) - wrm2);
			rdis = abs(pow(rm, 2) - rm2);
			printToFile(type, sizeType, count, size, rdis, read, write, srread, srwrite, wrdis, 1);
		}
	}
	else {
		bool flash = true;
		for (int j = 0; j < num; j++) {
			if (num > 1) {
				if (j == 1) {
					cout << "Запускать тест flash? 1 - да, 2 - нет" << endl;
					cin >> userInput;
					if (userInput == 2) {
						flash = false;
					}
				}
				type = memoryType[j];
				size = 4194304;
			}
			if ((type == "flash" || type == "HDD") && flash) {
				cout << "Введите адрес носителя:";
				cin >> address;
			}
			for (int k = 0; k < numStep && flash; k++) {
				for (int i = 0; i < count; i++) {
					testMemory(address, size, write[i], read[i]);
					wrm += write[i] / count;
					wrm2 += pow(write[i], 2) / count;
					rm += read[i] / count;
					rm2 += pow(read[i], 2) / count;
					srwrite += write[i];
					srread += read[i];
				}
				srwrite /= count;
				srread /= count;
				wrdis = abs(pow(wrm, 2) - wrm2);
				rdis = abs(pow(rm, 2) - rm2);
				printToFile(type, sizeType, count, size, rdis, read, write, srread, srwrite, wrdis, 1);
				if (numStep == 20) {
					size += 4194304;
					wrm2 = 0, wrm = 0, wrdis, rm = 0, rm2 = 0, rdis = 0, srwrite = 0, srread = 0;
				}
			}
		}
	}
}
