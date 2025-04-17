#include <iostream>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <string>
#include "fmod.hpp"
#include "fmod_errors.h"
#pragma comment(lib, "fmod_vc.lib")

const std::string MUSIC_FOLDER = "Music/";
const std::string FILE_EXTENSION = ".mp3";

#define ERRCHECK(_result) ERRCHECK_fn(_result, __FILE__, __LINE__)
void ERRCHECK_fn(FMOD_RESULT result, const char* file, int line) {
    if (result != FMOD_OK) {
        std::cerr << "FMOD error (" << file << ":" << line << ") - "
            << FMOD_ErrorString(result) << std::endl;
        exit(-1);
    }
}

std::vector<std::string> GetMusicFiles() {
    std::vector<std::string> files;
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((MUSIC_FOLDER + "*" + FILE_EXTENSION).c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: No MP3 files found in " << MUSIC_FOLDER << std::endl;
        return files;
    }

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            files.push_back(MUSIC_FOLDER + findData.cFileName);
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
    return files;
}

std::string SelectTrack(const std::vector<std::string>& files) {
    system("cls");
    std::cout << "Available tracks:\n";
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "[" << i + 1 << "] "
            << files[i].substr(MUSIC_FOLDER.length()) << "\n";
    }
    std::cout << "\n[Q] Quit\n";

    int choice;
    char input;
    do {
        std::cout << "Select track (1-" << files.size() << ") or Q to quit: ";
        std::cin >> input;

        if (toupper(input) == 'Q') return "";

        choice = input - '0';
    } while (choice < 1 || choice > static_cast<int>(files.size()));

    return files[choice - 1];
}

void Play3DSound(FMOD::System* system, const std::string& filePath) {
    FMOD::Sound* sound = nullptr;
    FMOD::Channel* channel = nullptr;
    FMOD_VECTOR sourcePos = { 0 };
    float angle = 0.0f;
    const float radius = 5.0f;
    const float speed = 0.05f;

    // Загрузка и воспроизведение
    ERRCHECK(system->createSound(filePath.c_str(), FMOD_3D | FMOD_LOOP_NORMAL, 0, &sound));
    ERRCHECK(system->playSound(sound, nullptr, true, &channel));

    channel->set3DMinMaxDistance(1.0f, 10000.0f);
    channel->set3DAttributes(&sourcePos, nullptr);
    channel->setPaused(false);

    // Цикл обработки
    std::cout << "\nNow playing: " << filePath.substr(MUSIC_FOLDER.length())
        << "\n[SPACE] Stop and choose new track\n[ESC] Exit\n";

    while (true) {
        // Обновление позиции
        sourcePos.x = radius * cos(angle);
        sourcePos.z = radius * sin(angle);
        channel->set3DAttributes(&sourcePos, nullptr);
        system->update();

        angle = fmod(angle + speed, 6.28318f);

        // Обработка ввода
        if (_kbhit()) {
            int key = _getch();
            if (key == 32) { // Пробел
                channel->stop();
                sound->release();
                return;
            }
            else if (key == 27) { // ESC
                channel->stop();
                sound->release();
                exit(0);
            }
        }
        Sleep(50);
    }
}

int main() {
    // Инициализация FMOD
    FMOD::System* system = nullptr;
    ERRCHECK(FMOD::System_Create(&system));
    ERRCHECK(system->init(32, FMOD_INIT_3D_RIGHTHANDED, 0));
    system->set3DSettings(1.0f, 1.0f, 1.0f);

    // Главный цикл
    while (true) {
        auto musicFiles = GetMusicFiles();
        if (musicFiles.empty()) {
            std::cout << "Press any key to exit...";
            _getch(); // Замена system("pause")
            return 1;
        }

        std::string selectedFile = SelectTrack(musicFiles);
        if (selectedFile.empty()) break;

        Play3DSound(system, selectedFile);
    }

    // Очистка
    system->close();
    system->release();
    return 0;
}