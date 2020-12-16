//Задача A “Сжатие с потерями” (6 баллов)
//Эту задачу не надо сдавать в контест.
//
//Изучите формат WAV-файлов: https://audiocoding.ru/articles/2008-05-22-wav-file-structure/

//Немного адаптированный код, считывающий wav, здесь: https://pastebin.com/cq77Tw6P

//Простенький сэмпл с одноканальным звуком и 16 битной глубиной можно взять здесь:
// https://drive.google.com/file/d/1iXKjVxfhlHrgapKE5cUvYeXwHZ5zNDV_/view?usp=sharing
//Но лучше найти свой wav :)

//Реализуйте прямое и обратное дискретное преобразование Фурье FFT и FFTReverse за O(n log n).
//По данным data из wav-файла вычислите g = FFT( data ).
//Обнулите некоторую долю D (например, 80%) последних коэффициентов g.
//Вычислите data2 = FFTReverse( g ).
//Сохраните wav файл, сравните на слух с исходным.

#include <iostream>
#include <complex>
#include <utility>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>


// Структура, описывающая заголовок WAV файла.
struct {
    // WAV-формат начинается с RIFF-заголовка:

    // Содержит символы "RIFF" в ASCII кодировке
    // (0x52494646 в big-endian представлении)
    char chunkId[4];

    // 36 + subchunk2Size, или более точно:
    // 4 + (8 + subchunk1Size) + (8 + subchunk2Size)
    // Это оставшийся размер цепочки, начиная с этой позиции.
    // Иначе говоря, это размер файла - 8, то есть,
    // исключены поля chunkId и chunkSize.
    unsigned int chunkSize;

    // Содержит символы "WAVE"
    // (0x57415645 в big-endian представлении)
    char format[4];

    // Формат "WAVE" состоит из двух подцепочек: "fmt " и "data":
    // Подцепочка "fmt " описывает формат звуковых данных:

    // Содержит символы "fmt "
    // (0x666d7420 в big-endian представлении)
    char subchunk1Id[4];

    // 16 для формата PCM.
    // Это оставшийся размер подцепочки, начиная с этой позиции.
    unsigned int subchunk1Size;

    // Аудио формат, полный список можно получить здесь http://audiocoding.ru/wav_formats.txt
    // Для PCM = 1 (то есть, Линейное квантование).
    // Значения, отличающиеся от 1, обозначают некоторый формат сжатия.
    unsigned short audioFormat;

    // Количество каналов. Моно = 1, Стерео = 2 и т.д.
    unsigned short numChannels;

    // Частота дискретизации. 8000 Гц, 44100 Гц и т.д.
    unsigned int sampleRate;

    // sampleRate * numChannels * bitsPerSample/8
    unsigned int byteRate;

    // numChannels * bitsPerSample/8
    // Количество байт для одного сэмпла, включая все каналы.
    unsigned short blockAlign;

    // Так называемая "глубиная" или точность звучания. 8 бит, 16 бит и т.д.
    unsigned short bitsPerSample;

    // Подцепочка "data" содержит аудио-данные и их размер.

    // Содержит символы "data"
    // (0x64617461 в big-endian представлении)
    char subchunk2Id[4];

    // numSamples * numChannels * bitsPerSample/8
    // Количество байт в области данных.
    unsigned int subchunk2Size;

    // Далее следуют непосредственно Wav данные.
} typedef WavHeader;


class WavFile{
    // Класс, отвечающий за чтение, храние wav файлов в оперативной памяти и запись их в файловую систему.

protected:
    WavHeader header{};
    std::vector<double> data;

public:
    explicit WavFile(const std::string & filename) {
        read_file_wav(filename.c_str());
    }
    WavFile(const WavHeader & header, std::vector<double> data) : header(header), data(std::move(data)) {}

    [[nodiscard]] std::vector<double> getData() const {
        return data;
    }

    [[nodiscard]] WavHeader getHeader() const {
        return header;
    }

    void printInfo() const {
        // Выводим полученные данные
        std::cout << header.chunkId[0] << header.chunkId[1] << header.chunkId[2] << header.chunkId[3] << std::endl;
        printf("Chunk size: %d\n", header.chunkSize);
        std::cout << header.format[0] << header.format[1] << header.format[2] << header.format[3] << std::endl;
        std::cout << header.subchunk1Id[0] << header.subchunk1Id[1] << header.subchunk1Id[2] << header.subchunk1Id[3] << std::endl;
        printf("SubChunkId1: %d\n", header.subchunk1Size);
        printf("Audio format: %d\n", header.audioFormat);
        printf("Channels: %d\n", header.numChannels);
        printf("Sample rate: %d\n", header.sampleRate);
        printf("Bits per sample: %d\n", header.bitsPerSample);
        std::cout << header.subchunk2Id[0] << header.subchunk2Id[1] << header.subchunk2Id[2] << header.subchunk2Id[3] << std::endl;

        // Посчитаем длительность воспроизведения в секундах
        double fDurationSeconds = 1.f * header.subchunk2Size / (header.bitsPerSample / 8) / header.numChannels / header.sampleRate;
        int iDurationMinutes = (int)floor(fDurationSeconds) / 60;
        fDurationSeconds = fDurationSeconds - (iDurationMinutes * 60);
        printf("Duration: %02d:%02.f\n", iDurationMinutes, fDurationSeconds);
    }

    void save(const std::string & filename) {
        save_file_wav(filename.c_str());
    }

private:
    // TODO use modern file read-write
    void read_file_wav(const char* filename)
    {
        FILE *file = fopen(filename, "rb");
        if (!file)
        {
            std::cout << "Failed open file";
            return;
        }

        fread(&header, sizeof(WavHeader), 1, file);

        double value;
        data.reserve(header.subchunk2Size);

        size_t samples = header.subchunk2Size / (header.bitsPerSample / 8);

        // Сложно сделать что-то лучше, учитывая, что необходимо читать блоками по 8, 16, 24,... бит,
        // которые образуют соответственно знаковое 8, 16, 24,...-битное число.
        switch(header.bitsPerSample) {
            case 8:
                for (size_t i = 0; i < samples; ++i) {
                    auto * val = new int8_t;
                    if (fread(val, 1, 1, file) == -1) break;
                    data.push_back(*val);
                    delete val;
                }
                break;
            case 16:
                for (size_t i = 0; i < samples; ++i) {
                    auto * val = new int16_t;
                    if (fread(val, 2, 1, file) == -1) break;
                    data.push_back(*val);
                    delete val;
                }
                break;
            case 32:
                for (size_t i = 0; i < samples; ++i) {
                    auto * val = new int32_t;
                    if (fread(val, 4, 1, file) == -1) break;
                    data.push_back(*val);
                    delete val;
                }
                break;
            case 64:
                for (size_t i = 0; i < samples; ++i) {
                    auto * val = new int64_t;
                    if (fread(val, 8, 1, file) == -1) break;
                    data.push_back(*val);
                    delete val;
                }
                break;
            default:
                char s[100];
                snprintf(s, 100, "%d bits per sample is unsupported", header.bitsPerSample);
                perror(s);
                break;
        }

        std::cout << "Data is successfully loaded." << std::endl;
        fclose(file);
    }

    void save_file_wav(const char* filename) {
        FILE *file = fopen(filename, "w");
        if (!file)
        {
            std::cout << "Failed write file";
            return;
        }

        fwrite(&header, sizeof(WavHeader), 1, file);

        // Опять же очень грустный код, потому что нам нужно записывать блоками
        // неизвестного в compile-time размера (битности)
        switch(header.bitsPerSample) {
            case 8:
                for (double i : data) {
                    auto temp = static_cast<int8_t>(round(i));
                    auto * val = &temp;
                    fwrite(val, 1, 1, file);
                }
                break;
            case 16:
                for (double i : data) {
                    auto temp = static_cast<int16_t>(round(i));
                    auto * val = &temp;
                    fwrite(val, 2, 1, file);
                }
                break;
            case 32:
                for (double i : data) {
                    auto temp = static_cast<int32_t>(round(i));
                    auto * val = &temp;
                    fwrite(val, 4, 1, file);
                }
                break;
            case 64:
                for (double i : data) {
                    auto temp = static_cast<int64_t>(round(i));
                    auto * val = &temp;
                    fwrite(val, 8, 1, file);
                }
                break;
            default:
                char s[100];
                snprintf(s, 100, "%d bits per sample is unsupported", header.bitsPerSample);
                perror(s);
                break;
        }

        std::cout << "Output file is successfully wrote." << std::endl;
        fclose(file);
    }
};

class WavFileChannel : public WavFile{
    // Класс для работы с wav файлами, поддерживающий работу с отдельными каналами.

    std::vector<std::vector<double>> channels;

public:
    [[maybe_unused]] explicit WavFileChannel(const std::string & filename) : WavFile(filename) {
        createChannels();
    };

    WavFileChannel(const WavHeader & header, std::vector<double> data) : WavFile(header, std::move(data)) {
        createChannels();
    };

    [[nodiscard]] std::vector<double> getChannel(size_t i) const {
        return channels[i];
    }

    void setChannel(std::vector<double> new_channel, size_t i, bool update = true) {
        channels[i] = std::move(new_channel);
        if (update) {
            updateData();
        }
    }

    void updateData() {
        // функция, необходимая для обновления данных файла после изменения каналов

        size_t n_of_blocks = header.subchunk2Size / header.blockAlign;

        for (size_t i_block = 0; i_block < n_of_blocks; ++i_block) {
            for (size_t i_channel = 0; i_channel < header.numChannels; ++i_channel) {
                data[i_channel + i_block * header.numChannels] = channels[i_channel][i_block];
            }
        }
    }

private:

    void createChannels(){
        // генерируем данные по-канально.

        channels.resize(header.numChannels);
        size_t n_of_blocks = header.subchunk2Size / header.blockAlign;
        for (size_t i_channel = 0; i_channel < header.numChannels; ++i_channel) {
            channels[i_channel].resize(n_of_blocks);
            for (size_t i_block = 0; i_block < n_of_blocks; ++i_block) {
                channels[i_channel][i_block] = data[i_channel + i_block * header.numChannels];
            }
        }

    }
};

class WavProcessor{
    // Класс, содержащий методы для редактирования wav файлов

private:
    typedef std::complex<double> base;

public:
    WavProcessor() = delete;

    static WavFileChannel compress(const WavFile & file, double rate = 1.0){
        // функция, создающая wav-файл, с rate долей занулённых гармоник (последних коэффициентов) в разложении Фурье данных файла

        return compress(WavFileChannel(file.getHeader(), file.getData()), rate);
    }
    static WavFileChannel compress(WavFileChannel file, double rate = 1.0){

        for (size_t i_channel = 0; i_channel < file.getHeader().numChannels; ++i_channel) {
            // выполним эти действия для всех каналов по отдельности.

            auto data = file.getChannel(i_channel);
            // дополняем длину до 2^n
            complete2(data, 0.0);

            // создаем массив комплексных чисел
            auto complex_data = makeComplex(data);

            // выполняем быстрое преобразование Фурье
            fft(complex_data, false);

            // обнуляем последнюю долю rate коэффициентов в разложении Фурье
            for (size_t i = rate * complex_data.size(); i < complex_data.size(); ++i) {
                complex_data[i] = 0;
            }

            // выполняем обратное быстрое преобразование Фурье
            fft(complex_data, true);

            // записываем измененные каналы файла
            file.setChannel(makeReal(complex_data), i_channel, false);
        }

        // обновляем основные данные файла, потому что каналы были изменены
        file.updateData();

        return file;
    }

private:

    template<typename T>
    static std::vector<base> makeComplex(const std::vector<T> & data) {
        std::vector<base> complex;
        complex.reserve(data.size());
        for (const auto & i : data) {
            complex.emplace_back(static_cast<base>(i));
        }
        return complex;
    }

    static std::vector<double> makeReal(const std::vector<base> & complex) {
        std::vector<double> real;
        real.reserve(complex.size());
        for (const auto & i : complex) {
            real.emplace_back(i.real());
        }
        return real;
    }
    template<class T>
    static void complete2(std::vector<T> & data, T new_item) {
        // Дополняет массив нулями до длины 2^n

        size_t n = 1;
        while (n < data.size()) {
            n <<= 1;
        }
        n <<= 1;
        data.resize(n, new_item);
    }

    static void fft (std::vector<base> & a, bool invert) {
        int n = (int) a.size();
        if (n == 1)  return;

        std::vector<base> a0 (n/2),  a1 (n/2);
        for (size_t i = 0, j = 0; i < n; i += 2, ++j) {
            a0[j] = a[i];
            a1[j] = a[i+1];
        }
        // Для достижения асимптотики O(n log n) воспользуемся принципом "разделяй и властвуй" и будем рекурсивно вызывать fft от половины данных.
        fft (a0, invert);
        fft (a1, invert);

        double angle = 2 * M_PI / n * (invert ? -1 : 1);
        base w (1),  w_n (cos(angle), sin(angle));
        for (size_t i = 0; i < n / 2; ++i) {
            a[i] = a0[i] + w * a1[i];
            a[i+n/2] = a0[i] - w * a1[i];

            if (invert) {
                a[i] /= 2;
                a[i+n/2] /= 2;
            }

            w *= w_n;
        }
    }
};

void createAndCompress(const std::string & input_filename, const std::string & output_filename, double rate) {
    // функция, создающая файл, compressed с указанным rate

    WavFile input(input_filename);

//    input.printInfo();

    WavFile output = WavProcessor::compress(input, rate);
    output.save(output_filename);
}

int main(int argc, char** argv) {
    const static double COMPRESS_RATE = 0.2;
    const static std::string OUT_PREFIX = "out_";

    std::string input_filename, output_filename;
    if (argc < 2) {
        while(std::cout << "Enter input_filename .wav filename: ", std::cin >> input_filename) {
            output_filename = OUT_PREFIX + input_filename;
            createAndCompress(input_filename, output_filename, COMPRESS_RATE);
        }
    }
    else {
        for (size_t i = 1; i < argc; ++i) {
            input_filename = argv[i];
            output_filename = OUT_PREFIX + input_filename;
            createAndCompress(input_filename, output_filename, COMPRESS_RATE);
        }
    }

    return 0;
}
