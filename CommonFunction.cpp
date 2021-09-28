#include "CommonFunction.h"

std::vector<std::vector<char> > CommonFunction::split(std::vector<char> buffer, std::vector<char> splitter)
{
    int index = 0;
    size_t splitter_size = splitter.size();
    std::vector<std::vector<char>> s_buffer;
    std::vector<char> temp;
    int previouse_index = 0;
    while (index < buffer.size())
    {
        bool contain = true;
        int i = 0;
        //check that all items of splitter exist in buffer
        for (; i < splitter_size && i + index < buffer.size(); i++)
        {
            if (buffer[i + index] != splitter[i])
            {
                contain = false;
                break;
            }
        }
        //if the index occure with splitter then copy to result
        if (contain)
        {
            //temp.resize(index - (previouse_index + splitter_size));
            //std::copy(buffer.data() + previouse_index, buffer.data() + index, temp.data());
            s_buffer.push_back(temp);
            previouse_index = index;
            index += splitter_size;
            temp.clear();
        }
        else
        {
            temp.push_back(buffer[index]);
            index++;
        }
    }
    if (temp.size() > 0)
    {
        s_buffer.push_back(temp);
    }
    return s_buffer;
}
