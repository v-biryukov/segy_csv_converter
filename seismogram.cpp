#include "seismogram.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <climits>
#include <algorithm>
#include <math.h>


template <typename T>
void swap_endian(T& pX)
{
    // should static assert that T is a POD
    char& raw = reinterpret_cast<char&>(pX);
    std::reverse(&raw, &raw + sizeof(T));
}

template <typename T>
T swap_endian_copy(T pX)
{
    swap_endian(pX);
    return pX;
}

std::vector<std::string> getNextLineAndSplitIntoTokens(std::istream& str, char delim = ';')
{
    std::vector<std::string>   result;
    std::string                line;
    std::getline(str,line);

    std::stringstream          lineStream(line);
    std::string                cell;

    while(std::getline(lineStream,cell, delim))
    {
        result.push_back(cell);
    }
    return result;
}

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| \\
// |||||||||||||||||||||||| Seismogramm ||||||||||||||||||||||||||||||||||| \\
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| \\



template<typename Scalar>
void Seismogramm<Scalar>::swap_header_endian()
{
    swap_endian<uint32>(header_data.job_id);
    swap_endian<uint32>(header_data.line_num);
    swap_endian<uint32>(header_data.reel_num);
    swap_endian<uint16>(header_data.num_of_traces_per_record);
    swap_endian<uint16>(header_data.num_of_auxiliary_traces_per_record);
    swap_endian<uint16>(header_data.sample_interval_reel);
    swap_endian<uint16>(header_data.sample_interval);
    swap_endian<uint16>(header_data.samples_per_trace_reel);
    swap_endian<uint16>(header_data.samples_per_trace);
    swap_endian<uint16>(header_data.data_sample_format);
}

template<typename Scalar>
void Seismogramm<Scalar>::swap_trace_header_endian(struct segy_trace_header * ptr_header)
{
    swap_endian<uint32>(ptr_header->trace_seq_num_line);
    swap_endian<uint32>(ptr_header->trace_seq_num_reel);
    swap_endian<uint32>(ptr_header->field_record_num);
    swap_endian<uint32>(ptr_header->trace_num_reel);
    swap_endian<uint16>(ptr_header->trace_id_code);
    swap_endian<uint32>(ptr_header->source_x);
    swap_endian<uint32>(ptr_header->source_y);
    swap_endian<uint32>(ptr_header->receiver_x);
    swap_endian<uint32>(ptr_header->receiver_y);
    swap_endian<uint16>(ptr_header->units_id);
    swap_endian<uint16>(ptr_header->num_of_samples);
    swap_endian<uint16>(ptr_header->sample_interval);
    swap_endian<uint32>(ptr_header->distance_from_source);
    swap_endian<uint16>(ptr_header->num_of_verticaly_summed_traces);
    swap_endian<uint16>(ptr_header->num_of_horizotally_summed_traces);
    swap_endian<uint16>(ptr_header->data_use);
}

template<typename Scalar>
void Seismogramm<Scalar>::swap_all_trace_headers_endian()
{
    for (IndexType i = 0; i < trace_header_data.size(); i++)
        swap_trace_header_endian(&trace_header_data[i]);
}

template<typename Scalar>
void Seismogramm<Scalar>::swap_data_endian()
{
    for (int i = 0; i < data.size(); i++)
        for (int j = 0; j < data[i].size(); j++)
        {
            swap_endian(data[i][j]);
        }
}


template<typename Scalar>
void Seismogramm<Scalar>::LoadSegY(const std::string& path, std::vector<Scalar>& times)
{
    std::ifstream inf;
    inf.open(path.data(), std::ios::binary);
    if (!inf)
    {
        std::cout << "Error in reading SEG-Y file." << std::endl;
        std::cout << "There is no such file: " << path << std::endl;
        std::exit(1);
    }
    // Loading Text Header
    char text_header[3200];
    inf.read(text_header, 3200);

    // Loading Binary Header
    inf.read(reinterpret_cast<char*>(&header_data), sizeof(header_data));
    swap_header_endian();

    // Loading Data and Trace Headers
    data.resize(header_data.num_of_traces_per_record);
    trace_header_data.resize(header_data.num_of_traces_per_record);
    for (int i = 0; i < header_data.num_of_traces_per_record; i++)
    {
        data[i].resize(header_data.samples_per_trace);
    }
    for (int i = 0; i < header_data.num_of_traces_per_record; i++)
    {
        inf.read(reinterpret_cast<char*>(&trace_header_data[i]), sizeof(trace_header_data[i]));
        inf.read(reinterpret_cast<char*>(data[i].data()), sizeof(float) * header_data.samples_per_trace);
    }
    swap_all_trace_headers_endian();
    swap_data_endian();

    // Setting Times
    times.resize(header_data.samples_per_trace);
    for (int i = 0; i < header_data.samples_per_trace; i++)
    {
        times[i] = header_data.sample_interval * 0.000001 * i;
    }
}

template<typename Scalar>
void Seismogramm<Scalar>::SaveSegY(const std::string& path, const std::vector<Scalar>& times, bool save_empty_headers)
{
    std::ofstream outf (path.data(), std::ios::out | std::ios::binary);
    if (!outf)
    {
        std::cout << "Error in writing SEG-Y file.\n";
        std::exit(1);
    }

    char text_header[3200];
    for (int i = 0; i < 3200; i++)
        text_header[i] = 0;

    char empty_bin_header[sizeof(segy_bin_header_data)];
    for (int i = 0; i < sizeof(segy_bin_header_data); i++)
        empty_bin_header[i] = 0;

    char empty_trace_header[sizeof(segy_trace_header)];
    for (int i = 0; i < sizeof(segy_trace_header); i++)
        empty_trace_header[i] = 0;

    outf.write(text_header, 3200);




    // Saving Binary Header
    swap_header_endian();
    if (save_empty_headers)
        outf.write(reinterpret_cast<char*>(&empty_bin_header), sizeof(empty_bin_header));
    else
        outf.write(reinterpret_cast<char*>(&header_data), sizeof(header_data));
    swap_header_endian();

    // Saving Data and Trace Headers
    swap_all_trace_headers_endian();
    swap_data_endian();
    for (uint32 i = 0; i <  header_data.num_of_traces_per_record; i++)
    {
        if (save_empty_headers)
            outf.write(reinterpret_cast<char*>(&empty_trace_header), sizeof(empty_trace_header));
        else
            outf.write(reinterpret_cast<char*>(&trace_header_data.at(i)), sizeof(trace_header_data.at(i)));

        outf.write(reinterpret_cast<char*>(data.at(i).data()), sizeof(float) * data.at(i).size());
    }
    swap_all_trace_headers_endian();
    swap_data_endian();

    outf.close();

    // saving the most important additional info if headers are set to 0
    if (save_empty_headers)
    {
        std::ofstream additionalf ((path + ".info.txt").data(), std::ios::out);
        if (!additionalf)
        {
            std::cout << "Error in writing SEG-Y file(additional info).\n";
            std::exit(1);
        }
        additionalf << "Number of traces = " <<  header_data.num_of_traces_per_record << std::endl;
        additionalf << "Number of samples = " <<  header_data.samples_per_trace << std::endl;
        additionalf << "Time step(in ms.) = " <<  header_data.sample_interval << std::endl;
        additionalf.close();
    }
}

template<typename Scalar>
void Seismogramm<Scalar>::AddValue(const Sample& value, IndexType detectorIndex)
{
    data[detectorIndex].push_back(value);
}

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| \\
// |||||||||||||||||||||||| CombinedSeismogramm ||||||||||||||||||||||||||||||||||| \\
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| \\


template <typename Scalar, int dims>
void CombinedSeismogramm<Scalar, dims>::Load(SeismoType type, std::vector<std::string> paths)
{
    seismogramms.resize(componentInfos.size());
    if (type == SEG_Y)
    {
        for (IndexType p = 0; p < paths.size(); ++p)
        {
            seismogramms[p].LoadSegY(paths[p], times);
        }
    }
    else if (type == CSV)
    {
        seismogramms.resize(paths.size() * dims);
        for (IndexType path_index = 0; path_index < paths.size(); path_index++)
        {
            // Calculating number of time steps and number of traces
            // ///////////////////////////////////////
            std::string filename = paths[path_index] + ".csv";
            IndexType num_of_times = -1;
            IndexType num_of_all_traces = 0;
            IndexType num_of_receivers;
            std::ifstream ifs0;
            ifs0.open(filename.c_str());
            if (!ifs0)
            {
                std::cout << "Error in reading CSV file." << std::endl;
                std::cout << "There is no such file: " << filename << std::endl;
                std::exit(1);
            }
            std::vector<std::string> line = getNextLineAndSplitIntoTokens(ifs0);
            num_of_all_traces = line.size() - 1;
            num_of_receivers = num_of_all_traces / dims;
            if (line.back() == "\r") num_of_all_traces -= 1;
            while (line.size() >= num_of_all_traces + 1)
            {
                line = getNextLineAndSplitIntoTokens(ifs0);
                num_of_times++;
            }
            ifs0.close();

            // Reading  data
            // ///////////////////////////////////////
            std::ifstream ifs;
            ifs.open(filename.c_str());
            getNextLineAndSplitIntoTokens(ifs);

            line = getNextLineAndSplitIntoTokens(ifs);

            for (int k = 0; k < dims; k++)
                seismogramms.at(dims * path_index + k).data.resize((line.size()-1)/dims);

            for (int trace_i = 0; trace_i < num_of_receivers; trace_i++)
            {
                for (int k = 0; k < dims; k++)
                    seismogramms.at(dims * path_index + k).data[trace_i].resize(num_of_times);
            }
            times.resize(num_of_times);

            IndexType time_i = 0;
            while (line.size() >= num_of_all_traces + 1)
            {
                times[time_i] = ::atof(line[0].c_str());
                for (int trace_i = 0; trace_i < num_of_receivers; trace_i++)
                {
                    seismogramms.at(dims * path_index + 0).data[trace_i][time_i] = ::atof(line[1 + trace_i * dims].c_str());
                    seismogramms.at(dims * path_index + 1).data[trace_i][time_i] = ::atof(line[2 + trace_i * dims].c_str());
                    if (dims >= 3)
                        seismogramms.at(dims * path_index + 2).data[trace_i][time_i] = ::atof(line[3 + trace_i * dims].c_str());
                }
                line = getNextLineAndSplitIntoTokens(ifs);
                time_i++;
            }

            // Interpolating results on eqidistant time grid
            // ///////////////////////////////////////

            Scalar interval = (times[num_of_times-1] - times[0]) / (num_of_times - 1);
            interpolate_data_on_equal_time_intervals(interval * interpolation_multiplier);
            num_of_times = times.size();
            interval = times[1] - times[0];


            // Set binary header data
            // ///////////////////////////////////////
            struct segy_bin_header_data header_data;
            header_data.sample_interval = (uint32)(floor(interval * 1000000.0 + 0.5));
            header_data.sample_interval_reel = header_data.sample_interval;


            header_data.job_id = 1;
            header_data.line_num = 1;
            header_data.reel_num = 1;
            header_data.num_of_traces_per_record = num_of_receivers;
            header_data.num_of_auxiliary_traces_per_record = 0;
            header_data.data_sample_format = 5;
            header_data.reel_num = 1;
            header_data.samples_per_trace = num_of_times;
            header_data.samples_per_trace_reel = header_data.samples_per_trace;
            for (int i = 0; i < 17; i++)
                header_data.other[i] = 0;
            for (int i = 0; i < 170; i++)
                header_data.reserve[i] = 0;

            for (IndexType k = 0; k < dims; k++)
            {
                seismogramms[dims*path_index + k].header_data = header_data;
            }



            // Set trace header data
            // ///////////////////////////////////////

            // Reading receivers data
            std::string filename_rec = paths[path_index] + ".receivers.csv";
            std::ifstream ifs_rec;
            ifs_rec.open(filename_rec.c_str());
            std::vector<Scalar> rec_x;
            std::vector<Scalar> rec_y;
            if (ifs_rec)
            {
                for (IndexType i = 0; i < num_of_receivers; i++)
                {
                    std::vector<std::string> v = getNextLineAndSplitIntoTokens(ifs_rec);
                    if (v.size() < 2)
                    {
                        std::cout << "Error while reading receivers data" << std::endl;
                    }
                    else
                    {
                        rec_x.push_back(::atof(v.at(0).c_str()));
                        rec_y.push_back(::atof(v.at(1).c_str()));
                    }
                }
            }
            else
            {
                std::cout << "Warning: no csv receivers data found" << std::endl;
                std::cout << "         All receivers positions have been set to (0.0, 0.0)" << std::endl;
                for (IndexType i = 0; i < num_of_receivers; i++)
                {
                    rec_x.push_back(0.0);
                    rec_y.push_back(0.0);
                }
            }

            // Reading source data
            std::string filename_source = paths[path_index] + ".source.csv";
            std::ifstream ifs_source;
            ifs_source.open(filename_source.c_str());
            Scalar source_x;
            Scalar source_y;
            if (ifs_source)
            {
                std::vector<std::string> v = getNextLineAndSplitIntoTokens(ifs_source);
                if (v.size() < 2)
                {
                    std::cout << "Error while reading source data" << std::endl;
                }
                else
                {
                    source_x = ::atof(v.at(0).c_str());
                    source_y = ::atof(v.at(1).c_str());
                }
            }
            else
            {
                std::cout << "Warning: no csv source data found" << std::endl;
                std::cout << "         Source position has been set to (0.0, 0.0)" << std::endl;
                source_x = 0.0;
                source_y = 0.0;
            }

            // Setting trace headers data

            for (IndexType k = 0; k < dims; k++)
            {
                seismogramms[dims*path_index + k].trace_header_data.resize(num_of_receivers);
                for (int i = 0; i < num_of_receivers; i++)
                {
                    seismogramms[dims*path_index + k].trace_header_data[i].trace_seq_num_line = i;
                    seismogramms[dims*path_index + k].trace_header_data[i].trace_seq_num_reel = i;
                    seismogramms[dims*path_index + k].trace_header_data[i].trace_id_code = i;
                    seismogramms[dims*path_index + k].trace_header_data[i].receiver_x = rec_x[i];
                    seismogramms[dims*path_index + k].trace_header_data[i].receiver_y = rec_y[i];
                    seismogramms[dims*path_index + k].trace_header_data[i].source_x = source_x;
                    seismogramms[dims*path_index + k].trace_header_data[i].source_y = source_y;
                    seismogramms[dims*path_index + k].trace_header_data[i].field_record_num = 1;
                    seismogramms[dims*path_index + k].trace_header_data[i].num_of_samples = header_data.samples_per_trace;
                    seismogramms[dims*path_index + k].trace_header_data[i].sample_interval = header_data.sample_interval;
                    seismogramms[dims*path_index + k].trace_header_data[i].trace_num_reel = 1;
                    seismogramms[dims*path_index + k].trace_header_data[i].units_id = 1;
                    seismogramms[dims*path_index + k].trace_header_data[i].distance_from_source =
                            uint32(sqrt(Scalar((rec_x[i]-source_x)*(rec_x[i]-source_x) + (rec_y[i]-source_y)*(rec_y[i]-source_y)) + 0.5));
                }
            }


        }
    }
}

template <typename Scalar, int dims>
void CombinedSeismogramm<Scalar, dims>::interpolate_data_on_equal_time_intervals(Scalar time_interval)
{
    for (IndexType seism_i = 0; seism_i < seismogramms.size(); seism_i++)
    {
        for (int trace_i = 0; trace_i < seismogramms[seism_i].data.size(); trace_i++)
        {
            Scalar cur_time = times[0];
            IndexType cur_index = 1;
            typename Seismogramm<Scalar>::Trace temp_trace;
            while (cur_time < times.back())
            {
                while (cur_index < seismogramms[seism_i].data[trace_i].size() && cur_time > times[cur_index])
                    cur_index++;
                // Linear approx:
                temp_trace.push_back(
                    ((cur_time - times[cur_index-1])*seismogramms[seism_i].data[trace_i][cur_index] +
                     (times[cur_index] - cur_time)  *seismogramms[seism_i].data[trace_i][cur_index-1]) /
                     (times[cur_index] - times[cur_index-1]));
                cur_time += time_interval;
            }
            seismogramms[seism_i].data[trace_i].resize(temp_trace.size());
            for (IndexType time_i = 0; time_i < temp_trace.size(); time_i++)
            {
                seismogramms[seism_i].data[trace_i][time_i] = temp_trace[time_i];
            }
        }
        seismogramms[seism_i].header_data.sample_interval = uint16(time_interval * 1000000);
    }
    times.resize(seismogramms[0].data[0].size());
    for (IndexType i = 1; i < times.size(); i++)
        times[i] = times[0] + time_interval * i;
}

template <typename Scalar, int dims>
void CombinedSeismogramm<Scalar, dims>::Save(SeismoType type, std::vector<std::string> paths)
{
    if (type == SEG_Y)
    {
        if (paths.size()> seismogramms.size())
        {
            std::cout << "Too many Seg-Y files to save!" << std::endl;
            std::exit(1);
        }
        for (IndexType i = 0; i < paths.size(); ++i)
        {
            seismogramms[i].SaveSegY(paths[i].data(), times);
        }
    }
    else if (type == CSV)
    {
        if (paths.size() * dims > 3 * seismogramms.size())
        {
            std::cout << "Too many Csv files to save!" << std::endl;
            std::exit(1);
        }

        Scalar interval = (times.back() - times.front()) / (times.size() - 1);
        interpolate_data_on_equal_time_intervals(interval * interpolation_multiplier);
        interval = times[1] - times[0];

        for (IndexType path_index = 0; path_index < paths.size(); path_index++)
        {
            // Saving data
            std::ofstream outf ((paths[path_index] + ".csv").c_str(), std::ios::out);
            outf << "Time;";
            for (int i = 0; i < seismogramms[0].data.size(); i++)
            {
                outf << "Vx (edge = " << i + 1 << ");";
                outf << "Vy (edge = " << i + 1 << ");";
                if (dims >= 3)
                    outf << "Vz (edge = " << i + 1 << ");";
            }
            outf << "\n";
            for (int i = 0; i < seismogramms[dims*path_index].data[0].size(); i++)
            {
                outf << times[i] << ";";
                for (int j = 0; j < seismogramms[dims*path_index].data.size(); j++)
                {
                    outf << seismogramms[dims*path_index + 0].data[j][i] << ";";
                    outf << seismogramms[dims*path_index + 1].data[j][i] << ";";
                    if (dims >= 3)
                        outf << seismogramms[dims*path_index + 2].data[j][i] << ";";
                }
                outf << "\n";
            }
            outf.close();
            // Saving receivers
            std::ofstream outf_res ((paths[path_index] + ".rec.txt").c_str(), std::ios::out);
            for (int i = 0; i < seismogramms[dims*path_index].trace_header_data.size(); i++)
            {
                outf_res << seismogramms[dims*path_index].trace_header_data[i].receiver_x << " " <<
                            seismogramms[dims*path_index].trace_header_data[i].receiver_y << "\n";
            }
            outf_res.close();
            // Saving explosion coords
            std::ofstream outf_expl ((paths[path_index] + ".expl.txt").c_str(), std::ios::out);
            outf_expl << seismogramms[dims*path_index].trace_header_data[0].source_x << " " <<
                         seismogramms[dims*path_index].trace_header_data[0].source_y << "\n";
            outf_expl.close();
        }
    }
}

template <typename Scalar, int dims>
void CombinedSeismogramm<Scalar, dims>::Save(SeismoType type)
{
    std::vector<std::string> paths;
    for (int k = 0; k < componentInfos.size(); k++)
        paths.push_back(componentInfos[k].path);
    Save(type, paths);
}

template <typename Scalar, int dims>
void CombinedSeismogramm<Scalar, dims>::AddValue(Scalar time, const Elastic& elastic, IndexType detectorIndex)
{
    times.push_back(time);
    for (IndexType i = 0; i < componentInfos.size(); ++i)
    {
        seismogramms[i].AddValue(componentInfos[i].getter->GetValue(elastic), detectorIndex);
    }
}

template <typename Scalar, int dims>
void CombinedSeismogramm<Scalar, dims>::AddComponent(const std::string& path, ValueGetter<Elastic, dims>* getter)
{
    componentInfos.push_back(ComponentInfo(path, getter));
}


template class Seismogramm<float>;

template class CombinedSeismogramm<float, 2>;
template class CombinedSeismogramm<float, 3>;

