#include "commands_functions.cpp"

int shortcut_filename_ext = 0;
#pragma region print segment summary
 void segmentsummaryprinting(std::string amt_str, std::string srt_st)
 {
    int begin = std::stoi(srt_st);
    int terminate = (std::stoi(amt_str) + begin < ASSIGNED_BLOCK) ? std::stoi(amt_str) + begin : ASSIGNED_BLOCK;
    for (int i = begin; i < terminate; ++i)
    {
    printf("Active Seg Summary [%d]: {%u, %u}\n", i, SUMMARY_OF_SEGMENT[i][0], SUMMARY_OF_SEGMENT[i][1]);
    }
 }
#pragma endregion print segment summary

#pragma region for restarting
 void restartagain()
 {
    CheckpointRegionReading();
    findadjacentblockavilable();
    readinginsidesegment();
    Imapreading();
 }
#pragma endregion for restarting

#pragma region imap
  void imapcontent(std::string amt_str, std::string srt_st)
 {
    int begin = std::stoi(srt_st);
    int terminate = (std::stoi(amt_str) + begin < MAXIMUM_FILES) ? std::stoi(amt_str) + begin : MAXIMUM_FILES;
    for (int i = begin; i < terminate; ++i)
    {
      printf("IMAP[%d]: %u\n", i, IMAP[i]);
    }
 }
#pragma endregion imap

#pragma region import number of time same file
  void importntimes(std::string filename, std::string lfs_filename, std::string amt_str) 
 {
    int amount = (std::stoi(amt_str) < MAXIMUM_FILES) ? std::stoi(amt_str) : MAXIMUM_FILES;
    for (int i = 0; i < amount; ++i)
    {
    import(filename, lfs_filename+std::to_string(i));
    }
 }
#pragma endregion import number of time same file

#pragma region clean
 void printCleans()
 {
    printf("segment that are clean : ");
    for (int i = 0; i < NUMBER_OF_SEGMENT; ++i)
    {
      if (NUMBER_OF_CLEAN_SEGMENT[i] == CLEAN_SEG)
      {
          printf("%d ", i+1);
      }
    }
    printf("\n");
 }
#pragma endregion clean 

#pragma region checkpoint
 void printCheckpointRegion()
 {
    for (int i = 0; i < IMAP_BLKS; ++i)
  {
    printf("Checkpoint[%d]: %u\n", i, CHKPOINT_REG[i]);
  }
 }
#pragma region checkpoint

 void parseLine(std::string line) 
 {
    if (line.length() == 0) return;
    std::vector<std::string> command = split(line);
    if      (command[0] == "import"               && command.size() == 3) import(command[1], command[2]);
    else if (command[0] == "deletefile"               && command.size() == 2) deletefile(command[1]);
    else if (command[0] == "containoffile"        && command.size() == 2) containoffile(command[1]);
    else if (command[0] == "display"                  && command.size() == 4) display(command[1], command[2], command[3]);
    else if (command[0] == "change"                && command.size() == 5) change(command[1], command[2], command[3], command[4]);
    else if (command[0] == "clean"                 && command.size() == 2) clean(command[1]);
    else if (command[0] == "exit"                 && command.size() == 1) exit();
    else if (command[0] == "ls"                   && command.size() == 1) list();
    else if (command[0] == "currentsegment"        && command.size() == 1) printf("Current Segment: %d\n", SEGMENT_NUMBER);
    else if (command[0] == "segmentsummary"       && command.size() == 3) segmentsummaryprinting(command[1], command[2]);
    else if (command[0] == "importntimes"          && command.size() == 4) importntimes(command[1], command[2], command[3]);
    else if (command[0] == "imap"                 && command.size() == 3) imapcontent(command[1], command[2]);
    else if (command[0] == "checkpoint"            && command.size() == 1) printCheckpointRegion();
    else if (command[0] == "cleansegments"        && command.size() == 1) printCleans();
    else if (command[0] == "nextblock"             && command.size() == 1) printf("Next Available Block: %u\n", BLOCKS_THAT_ARE_AVILABLE);
    else if (command[0] == "clear"                 && command.size() == 1) std::system("clear");
    else std::cout << "Command not recognized." << std::endl;
 }

// Driver code 
 int main(int argc, char* argv[])
 {
    struct stat st = {0};
    if (stat("./SegmentChaeckPointDrive", &st) == -1)
    {
        std::cout << "Could not find DRIVE. Please run 'make drive' and try again." << std::endl;
        return 1;
    }
    restartagain();
    std::string line;
    std::cout << "lfs>>";
    while (getline(std::cin, line))
    {
    parseLine(line);
    std::cout << "lfs>>";
    }
    return 0;
}
