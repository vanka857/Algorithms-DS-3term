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
#include <fstream>
#include <complex>
#include <vector>
#include <cmath>
#include <string>
#include <cstring>
#include <algorithm>
#include <cassert>


// Структура, описывающая заголовок WAV файла.
struct {
    // WAV-формат начинается с RIFF-заголовка:

    // Содержит символы "RIFF" в ASCII кодировке
    // (0x52494646 в big-endian представлении)
    char chunk_id[4];

    // 36 + subchunk_2_size, или более точно:
    // 4 + (8 + subchunk_1_size) + (8 + subchunk_2_size)
    // Это оставшийся размер цепочки, начиная с этой позиции.
    // Иначе говоря, это размер файла - 8, то есть,
    // исключены поля chunk_id и chunk_size.
    unsigned int chunk_size;

    // Содержит символы "WAVE"
    // (0x57415645 в big-endian представлении)
    char format[4];

    // Формат "WAVE" состоит из двух подцепочек: "fmt " и "data":
    // Подцепочка "fmt " описывает формат звуковых данных:

    // Содержит символы "fmt "
    // (0x666d7420 в big-endian представлении)
    char subchunk_1_id[4];

    // 16 для формата PCM.
    // Это оставшийся размер подцепочки, начиная с этой позиции.
    unsigned int subchunk_1_size;

    // Аудио формат, полный список можно получить здесь http://audiocoding.ru/wav_formats.txt
    // Для PCM = 1 (то есть, Линейное квантование).
    // Значения, отличающиеся от 1, обозначают некоторый формат сжатия.
    unsigned short audio_format;

    // Количество каналов. Моно = 1, Стерео = 2 и т.д.
    unsigned short num_channels;

    // Частота дискретизации. 8000 Гц, 44100 Гц и т.д.
    unsigned int sample_rate;

    // sample_rate * num_channels * bits_per_sample/8
    unsigned int byte_rate;

    // num_channels * bits_per_sample/8
    // Количество байт для одного сэмпла, включая все каналы.
    unsigned short block_align;

    // Так называемая "глубиная" или точность звучания. 8 бит, 16 бит и т.д.
    unsigned short bits_per_sample;

    // Подцепочка "data" содержит аудио-данные и их размер.

    // Содержит символы "data"
    // (0x64617461 в big-endian представлении)
    char subchunk_2_id[4];

    // numSamples * num_channels * bits_per_sample/8
    // Количество байт в области данных.
    unsigned int subchunk_2_size;

    // Далее следуют непосредственно Wav данные.
} typedef WavHeader;

// Класс, содержащий методы для редактирования wav файлов
class WavProcessor;
// Класс, отвечающий за чтение, храние wav файлов в оперативной памяти и запись их в файловую систему.
class WavFile;

// Класс, отвечающий за чтение, храние wav файлов в оперативной памяти и запись их в файловую систему.
class WavFile{
    friend WavProcessor;

private:
    WavHeader header{};
    std::vector<std::vector<double>> channels;
    size_t bytes_per_sample = 16;
    size_t n_of_blocks = 0;

public:
    explicit WavFile(const std::string & filename) {
        read(filename);
    }

    void printInfo() const {
        // Выводим полученные данные
        std::cout << header.chunk_id[0] << header.chunk_id[1] << header.chunk_id[2] << header.chunk_id[3] << std::endl;
        printf("Chunk size: %d\n", header.chunk_size);
        std::cout << header.format[0] << header.format[1] << header.format[2] << header.format[3] << std::endl;
        std::cout << header.subchunk_1_id[0] << header.subchunk_1_id[1] << header.subchunk_1_id[2] << header.subchunk_1_id[3] << std::endl;
        printf("SubChunkId1: %d\n", header.subchunk_1_size);
        printf("Audio format: %d\n", header.audio_format);
        printf("Channels: %d\n", header.num_channels);
        printf("Sample rate: %d\n", header.sample_rate);
        printf("Bits per sample: %d\n", header.bits_per_sample);
        std::cout << header.subchunk_2_id[0] << header.subchunk_2_id[1] << header.subchunk_2_id[2] << header.subchunk_2_id[3] << std::endl;

        // Посчитаем длительность воспроизведения в секундах
        double fDurationSeconds = static_cast<double>(header.subchunk_2_size) / (header.bits_per_sample / 8) / header.num_channels / header.sample_rate;
        int iDurationMinutes = (int)floor(fDurationSeconds) / 60;
        fDurationSeconds = fDurationSeconds - (iDurationMinutes * 60);
        printf("Duration: %02d:%02.f\n", iDurationMinutes, fDurationSeconds);
    }

    void save(const std::string & filename) {
        write(filename);
    }

private:

    double readIntSample(std::ifstream & file) const {
        // Используем магию приведения типов указателей Little-Endian.

        // 8 - максимальное количество байт в сэмпле аудиофайла
        const static size_t MAX_BYTE_PER_SAMPLE = 8;

        char buf[MAX_BYTE_PER_SAMPLE];
        memset(buf, 0, bytes_per_sample);

        file.read(buf, bytes_per_sample);

        bool sign = buf[bytes_per_sample - 1] >> 7; // смотрим на знаковый бит числа в Little-Endian
        uint8_t end_template = sign ? 0b11111111 : 0;
        // дополняем число в конце единичками "11111111" или нулями "00000000" для
        // корректного представления отрицательных чисел
        size_t end_len = MAX_BYTE_PER_SAMPLE - bytes_per_sample;
        memset(buf + bytes_per_sample, end_template, end_len);

        return *reinterpret_cast<int64_t*>(buf);
    }
    double readSample(std::ifstream & file) const {
        switch (header.bits_per_sample) {
            case 8:
                // uint8_t data (unsupported)
                perror("Unsupported bits per sample. Only int16, int24, int32");
                break;
            case 16:
                // int16_t data
                return readIntSample(file);
                break;
            case 24:
                // int24_t data
                return readIntSample(file);
                break;
            case 32:
                // int24_t data
                // of float32 data (unsupported)
                return readIntSample(file);
                break;
            case 64:
                // float64 data (unsupported)
                perror("Unsupported bits per sample. Only int16, int24, int32");
                break;
            default:
                perror("Unsupported bits per sample. Only int16, int24, int32");
        }
        return 0;
    }

    void readHeader(std::ifstream & file) {
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
    }

    void readData(std::ifstream & file) {
        assert(("I do not support more than 8 bytes per sample!", header.bits_per_sample / 8 <= 8));

        channels.resize(header.num_channels);

        for (size_t i_block = 0; i_block < n_of_blocks; ++i_block) {
            for (size_t i_channel = 0; i_channel < header.num_channels && i_channel < channels.size(); ++i_channel) {
                channels[i_channel].push_back(readSample(file));
            }
        }
    }

    void read(const std::string & filename)
    {
        std::ifstream file(filename, std::ios_base::in | std::ios_base::binary);
        readHeader(file);

        bytes_per_sample = header.bits_per_sample / 8;
        n_of_blocks = header.subchunk_2_size / header.block_align;
        readData(file);

        file.close();
    }

    void writeIntSample(std::ofstream & file, double sample_data) const {
        // Используя приведение типов указателей, записываем данные в виде char-ов

        // Так как данные хранятся и записываются в формате Little-Endian, это законно

        int64_t sample_data_rounded = round(sample_data);
        file.write(reinterpret_cast<char*>(&sample_data_rounded), bytes_per_sample);
    }
    void writeSample(std::ofstream & file, double sample_data) const {
        // Будем поддерживать максимум 64 битные сэмплы.
        switch (header.bits_per_sample) {
            case 8:
                // uint8_t data (unsupported)
                perror("Unsupported bits per sample. Only int16, int24, int32");
                break;
            case 16:
                // int16_t data
                return writeIntSample(file, sample_data);
                break;
            case 24:
                // int24_t data
                return writeIntSample(file, sample_data);
                break;
            case 32:
                // int24_t data
                // of float32 data (unsupported)
                return writeIntSample(file, sample_data);
                break;
            case 64:
                // float64 data (unsupported)
                perror("Unsupported bits per sample. Only int16, int24, int32");
                break;
            default:
                perror("Unsupported bits per sample. Only int16, int24, int32");
        }
    }
    void write(const std::string & filename) {
        std::ofstream file(filename, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

        file.write(reinterpret_cast<char*>(&header), sizeof(header));

        for (size_t i_block = 0; i_block < n_of_blocks; ++i_block) {
            for (size_t i_channel = 0; i_channel < header.num_channels && i_channel < channels.size(); ++i_channel) {
                writeIntSample(file, channels[i_channel][i_block]);
            }
        }

        file.close();
    }
};

// Класс, содержащий методы для редактирования wav файлов
class WavProcessor{
    typedef std::complex<double> base;

public:
    WavProcessor() = delete;

    static WavFile compress(WavFile file, double rate = 1.0) {

        for (size_t i_channel = 0; i_channel < file.header.num_channels; ++i_channel) {
            // выполним эти действия для всех каналов по отдельности.

            auto & data = file.channels[i_channel];

            size_t len = data.size();

            // дополняем длину до 2^n
            complete2n(data, 0.0);

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

            // получаем вектор действительных чисел
            auto real = makeReal(complex_data);

            // записываем измения в файл (пишем только нужный изначальный размер, помня, что сы увеличивали длину)
            data = std::vector<double>(real.begin(), real.begin() + len);
        }
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
    static void complete2n(std::vector<T> & data, T new_item) {
        // Дополняет массив до длины 2^n элементами new_item

        size_t n = 1;
        while (n < data.size()) {
            n <<= 1;
        }
        data.resize(n, new_item);
    }

    static void fft (std::vector<base> & a, bool invert) {
        size_t n = a.size();
        if (n == 1){
            return;
        }

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

    input.printInfo();

    WavFile output = WavProcessor::compress(input, rate);
    output.save(output_filename);

    std::cout << output_filename << " saved!\n" << std::endl;
}

int main(int argc, char** argv) {
    const static double COMPRESS_RATE = 0.05;
    const static std::string OUT_PREFIX = "out_";

    std::string input_filename, output_filename;
    if (argc < 3) {
        while(std::cout << "Enter .wav filename: ", std::cin >> input_filename) {
            double rate;

            std::cout << "Enter rate: ";
            std::cin >> rate;
            output_filename = OUT_PREFIX + input_filename;
            createAndCompress(input_filename, output_filename, rate);
        }
    }
    else {
        for (size_t i = 2; i < argc; ++i) {
            double rate = strtod(argv[1], nullptr);
            std::cout << "Rate: " << rate;
            input_filename = argv[i];
            std::cout << " file: " << input_filename << std::endl;
            output_filename = OUT_PREFIX + input_filename;
            createAndCompress(input_filename, output_filename, rate);
        }
    }

    return 0;
}
