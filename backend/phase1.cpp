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
    char IR[4]; // Instruction Register sa
    char R[4];  // General purpose Register
    bool C;     //  Toggle Register
    int IC;     // Instruction counter
    int SI;     // System Interrupt
    stringstream infile;  // Changed from fstream to stringstream
    stringstream outfile; // Changed from ofstream to stringstream
    string outputContent; // Store output for API response

    // Initialize all the variables (buffer, memory, IR, R, IC, C, SI)
    void init()
    {
        fill(buffer, buffer + sizeof(buffer), '\0');
        fill(&Memory[0][0], &Memory[0][0] + sizeof(Memory), '\0');
        fill(IR, IR + sizeof(IR), '\0');
        fill(R, R + sizeof(R), '\0');
        IC = 0;
        C = true;
        SI = 0;
        outputContent = ""; // Initialize output content
    }

    // Reset buffer to null
    void resetBuffer()
    {
        fill(buffer, buffer + sizeof(buffer), '\0');
    }

    // Master Mode
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
        if (infile.str().empty())
            return;

        string line;
        while (getline(infile, line))
        {

            // --- START OF JOB ---
            if (line.substr(0, 4) == "$AMJ")
            {
                init();
                outputContent += "New Job started\n";
                // Reset output stream
                outfile.str("");
                outfile.clear();
            }

            // --- START OF DATA ---
            else if (line.substr(0, 4) == "$DTA")
            {
                outputContent += "Data card loading\n";
                resetBuffer();
                STARTEXE();
            }

            // --- END OF JOB ---
            else if (line.substr(0, 4) == "$END")
            {
                outputContent += "END of Job\n";
            }

            // -- PROGRAM CARD --
            else
            {
                outputContent += "Program Card loading\n";

                stringstream ss(line);
                string instr;

                // Split line into instructions like "GD20", "PD20", "H"
                while (ss >> instr)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        Memory[IC][j] = (j < instr.size()) ? instr[j] : '\0';
                    }
                    IC++;
                }

                // Debug: print just the loaded instructions
                for (int i = 0; i < IC; i++)
                {
                    outputContent += "M[" + to_string(i) + "] ";
                    for (int j = 0; j < 4; j++)
                        outputContent += Memory[i][j];
                    outputContent += "\n";
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

        string data;
        if (getline(infile, data)) {
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
                    outfile << Memory[i][j];
                    outputContent += Memory[i][j]; // Also add to output content
                }
            }
        }
        outfile << "\n";
        outputContent += "\n";
    }

    void TERMINATE()
    {
        outputContent += "Terminate called\n\n";
        outfile << "\n\n";
        outputContent += "\n\n";
    }

    void EXECUTEUSERPROGRAM()
    { // Slave Mode
        while (IC < 99 && Memory[IC][0] != '\0')
        {

            for (int i = 0; i < 4; i++)
            {
                IR[i] = Memory[IC][i];
            }

            IC++;

            // SI= 1-GD, 2-PD, 3-H

            // GD
            if (IR[0] == 'G' && IR[1] == 'D')
            {
                SI = 1;
                MOS();
            }

            // PD
            else if (IR[0] == 'P' && IR[1] == 'D')
            {
                SI = 2;
                MOS();
            }

            // H
            else if (IR[0] == 'H')
            {
                SI = 3;
                MOS();
                return;
            }

            // LR - LOAD DATA (R <-- memory[IR[2,3]])
            else if (IR[0] == 'L' && IR[1] == 'R')
            {
                for (int i = 0; i < 4; i++)
                {
                    R[i] = Memory[(IR[2] - '0') * 10 + (IR[3] - '0')][i];
                }
            }

            // SR - STORE (memory[IR[2,3]] <-- R)
            else if (IR[0] == 'S' && IR[1] == 'R')
            {
                for (int i = 0; i < 4; i++)
                {
                    Memory[(IR[2] - '0') * 10 + (IR[3] - '0')][i] = R[i];
                }
            }

            // CR - COMPARE(R, memory[IR[2,3]])
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
                if (cnt == 4)
                {
                    C = true;
                }
                else
                {
                    C = false;
                }
            }

            // BT (JUMP if toogle is T)
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
    // Original constructor for file-based execution
    VM()
    {
        // infile.open("./example_job.txt", ios::in);
        // infile.open("./input_custom.txt", ios::in);
        ifstream file("./input_Phase1.txt", ios::in);
        if (file.is_open()) {
            string content((istreambuf_iterator<char>(file)), 
                          istreambuf_iterator<char>());
            infile.str(content);
            file.close();
        }
        init();
        LOAD();
    }

    // New constructor for API-based execution
    VM(const string& inputContent)
    {
        infile.str(inputContent);
        init();
        LOAD();
    }

    // Method to get output for API response
    string getOutput() const {
        return outputContent;
    }
};

// Original main function for file-based execution
int main_original()
{
    VM v;
    return 0;
}

// New main function for CLI execution with backend integration
int main(int argc, char* argv[])
{
    if (argc == 1) {
        // No arguments - run original behavior
        VM v;
        return 0;
    }
    else if (argc == 3) {
        // CLI mode: phase1.exe input.txt output.txt
        ifstream inputFile(argv[1]);
        if (!inputFile.is_open()) {
            cerr << "Error: Cannot open input file " << argv[1] << endl;
            return 1;
        }
        
        string content((istreambuf_iterator<char>(inputFile)), 
                      istreambuf_iterator<char>());
        inputFile.close();
        
        VM vm(content);
        
        ofstream outputFile(argv[2]);
        if (!outputFile.is_open()) {
            cerr << "Error: Cannot open output file " << argv[2] << endl;
            return 1;
        }
        
        outputFile << vm.getOutput();
        outputFile.close();
        
        return 0;
    }
    else {
        cerr << "Usage: " << argv[0] << " [input_file output_file]" << endl;
        cerr << "If no arguments provided, uses default input_Phase1.txt" << endl;
        return 1;
    }
}