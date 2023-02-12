#pragma once
#include "Board.h"

#if defined LOG_TO_FILE 
#include <cstdio>
#include <fstream>
#include <ctime>

FILE* logger;

void setupFileLogging() {
	std::time_t t = std::time(0);
	std::tm now;
	localtime_s(&now, &t);
	std::string fileName = "EngineLog "
		+ std::to_string(now.tm_year + 1900) + '-'
		+ std::to_string(now.tm_mon + 1) + '-'
		+ std::to_string(now.tm_mday) + ' '
		+ std::to_string(now.tm_hour + 1) + '-'
		+ std::to_string(now.tm_min + 1) + '-'
		+ std::to_string(now.tm_sec + 1) + ".txt";
	std::ofstream file(fileName.c_str());
	if (file.good()) {
		file.close();
		freopen_s(&logger, fileName.c_str(), "w", stdout);
	}
	else {
		std::cerr << file.rdstate();
		throw std::logic_error("creating a logger failed");
	}
}
#endif

void initalizeEngine() {
#if defined LOG_TO_FILE 
	setupFileLogging();
#endif
#if defined DEBUG
	std::cout << "ENGINE >> started initializing engine" << std::endl;
#endif
	initalizeBoardClass();
#if defined DEBUG
	std::cout << "ENGINE >> finished initializing engine" << std::endl;
#endif
}
