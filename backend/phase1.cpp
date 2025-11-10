#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

class VM
{
private:
    char buffer[40];
    char Memory[100][4];
    char IR[4];
    char R[4];
    bool C;
    int IC;
    int SI;
    string inputContent;
    string outputContent;

    void init()
    {
        fill(buffer, buffer + sizeof(buffer), '\0');
        fill(&Memory[0][0], &Memory[0][0] + sizeof(Memory), '\0');
        fill(IR, IR + sizeof(IR), '\0');
        fill(R, R + sizeof(R), '\0');
        IC = 0;
        C = true;
        SI = 0;
        outputContent = "";
    }

    void resetBuffer()
    {
        fill(buffer, buffer + sizeof(buffer), '\0');
    }

    void MOS()
    {
        switch (SI)
        {
        case 1:
            READ();
            break;
        case 2:
            WRITE();
            break;
        case 3:
            TERMINATE();
            break;
        }
        SI = 0;
    }

    void LOAD()
    {
        istringstream infile(inputContent);
        string line;
        
        while (getline(infile, line))
        {
            if (line.substr(0, 4) == "$AMJ")
            {
                init();
                outputContent += "New Job started\n";
            }
            else if (line.substr(0, 4) == "$DTA")
            {
                outputContent += "Data card loading\n";
                resetBuffer();
                STARTEXE();
            }
            else if (line.substr(0, 4) == "$END")
            {
                outputContent += "END of Job\n";
            }
            else
            {
                outputContent += "Program Card loading\n";
                stringstream ss(line);
                string instr;

                while (ss >> instr)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        Memory[IC][j] = (j < instr.size()) ? instr[j] : '\0';
                    }
                    IC++;
                }
            }
        }
    }

    void STARTEXE()
    {
        IC = 0;
        EXECUTEUSERPROGRAM();
    }

    void READ()
    {
        outputContent += "Read function called\n";
        
        istringstream infile(inputContent);
        string data;
        // Skip to data section
        while (getline(infile, data)) {
            if (data.substr(0, 4) == "$DTA") {
                getline(infile, data);
                break;
            }
        }
        
        int len = data.size();
        for (int i = 0; i < len; i++)
        {
            buffer[i] = data[i];
        }
        int buff = 0, mem_ptr = (IR[2] - '0') * 10;
        while (buff < 40 && buffer[buff] != '\0')
        {
            for (int i = 0; i < 4; i++)
            {
                Memory[mem_ptr][i] = buffer[buff];
                buff++;
            }
            mem_ptr++;
        }
        resetBuffer();
    }

    void WRITE()
    {
        outputContent += "Write function called\n";
        
        for (int i = (IR[2] - '0') * 10; i < (IR[2] - '0' + 1) * 10; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (Memory[i][j] != '\0')
                {
                    outputContent += Memory[i][j];
                }
            }
        }
        outputContent += "\n";
    }

    void TERMINATE()
    {
        outputContent += "Terminate called\n\n";
    }

    void EXECUTEUSERPROGRAM()
    {
        while (IC < 99 && Memory[IC][0] != '\0')
        {
            for (int i = 0; i < 4; i++)
            {
                IR[i] = Memory[IC][i];
            }
            IC++;

            if (IR[0] == 'G' && IR[1] == 'D')
            {
                SI = 1;
                MOS();
            }
            else if (IR[0] == 'P' && IR[1] == 'D')
            {
                SI = 2;
                MOS();
            }
            else if (IR[0] == 'H')
            {
                SI = 3;
                MOS();
                return;
            }
            else if (IR[0] == 'L' && IR[1] == 'R')
            {
                for (int i = 0; i < 4; i++)
                {
                    R[i] = Memory[(IR[2] - '0') * 10 + (IR[3] - '0')][i];
                }
            }
            else if (IR[0] == 'S' && IR[1] == 'R')
            {
                for (int i = 0; i < 4; i++)
                {
                    Memory[(IR[2] - '0') * 10 + (IR[3] - '0')][i] = R[i];
                }
            }
            else if (IR[0] == 'C' && IR[1] == 'R')
            {
                int cnt = 0;
                for (int i = 0; i < 4; i++)
                {
                    if (Memory[(IR[2] - '0') * 10 + (IR[3] - '0')][i] == R[i])
                    {
                        cnt++;
                    }
                }
                C = (cnt == 4);
            }
            else if (IR[0] == 'B' && IR[1] == 'T')
            {
                if (C)
                {
                    IC = (IR[2] - '0') * 10 + (IR[3] - '0');
                }
            }
        }
    }

public:
    string execute(const string& input)
    {
        inputContent = input;
        init();
        LOAD();
        return outputContent;
    }
};

// For direct execution
int main(int argc, char* argv[])
{
    if (argc != 3) {
        cout << "Usage: phase1 <input_file> <output_file>" << endl;
        return 1;
    }
    
    ifstream infile(argv[1]);
    ofstream outfile(argv[2]);
    
    if (!infile.is_open()) {
        cout << "Error opening input file" << endl;
        return 1;
    }
    
    string content((istreambuf_iterator<char>(infile)), 
                   istreambuf_iterator<char>());
    
    VM vm;
    string result = vm.execute(content);
    outfile << result;
    
    infile.close();
    outfile.close();
    return 0;
}