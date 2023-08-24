#include "func.cpp"
#pragma region drive
//Make the hard drive
  void makethedirectiveDrive()
  {
       struct stat st = {0};

       if (stat("./SegmentChaeckPointDrive", &st) == -1)
       mkdir("./SegmentChaeckPointDrive", 0700);
  }
//End -Make the hard drive
#pragma endregion drive
//Divide the disk into 32 segments
#pragma region segment
  void Segmentintialization(){
       std::ofstream outs[NUMBER_OF_SEGMENT];
       for (int i = 0; i < NUMBER_OF_SEGMENT; ++i)
      {
            outs[i].open("SegmentChaeckPointDrive/SEGMENT"+std::to_string(i+1), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
            char true_zero[1] = {0};
            for (int j = 0; j < ASSIGNED_BLOCK * SIZE_OF_BLOCK; ++j)
            {
                outs[i].write(true_zero, 1);
            }
            unsigned int neg1 = -1;
            for (int j = 0; j < BLOCK_PER_SEGMENT * 2; ++j)
            {
                 outs[i].write(reinterpret_cast<const char*>(&neg1), 4);
            }
            outs[i].close();
      }
  }
#pragma endregion segment
//End - Divide the disk into 32 segments
// Creating block for checkpoint region
#pragma region checkpoint
  void checkpointintialization()
  {
       std::ofstream out;
       out.open("SegmentChaeckPointDrive/CHECKPOINT_REGION", std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
       unsigned int neg1 = -1;
       for (int i = 0; i < IMAP_BLKS; ++i)
       {
            out.write(reinterpret_cast<const char*>(&neg1), 4);
       }
       char true_zero[1] = {0};
       for (int i = 0; i < NUMBER_OF_SEGMENT; ++i)
       {
            out.write(true_zero, 1);
       }
       out.close();
  }
#pragma endregion checkpoint
//End - Creating block for checkpoint region
// Creating block for storing filemap
#pragma region filemapping
  void intializationblockforfilemapping()
  {
       std::ofstream out;
       out.open("SegmentChaeckPointDrive/FILEMAP", std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
       char true_zero = 0;
       for (int i = 0; i < FILEMAP_SIZE_OF_BLOCK * MAXIMUM_FILES; ++i)
       {
            out << true_zero;
       }
       out.close();
  }
#pragma endregion filemapping
//End - Creating block for storing filemap

//driver code 
 int main(int argc, char const *argv[]){
       makethedirectiveDrive(); //create hard drive
       Segmentintialization();  //divide disk into segments 
       checkpointintialization(); //crate block for checkpoint region
       intializationblockforfilemapping(); // create block to store filemap in each inode
       printf("%s\n", "Segment & CheckPoint drive created.");
  return 0;
 }
