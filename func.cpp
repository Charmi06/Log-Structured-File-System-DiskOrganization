#include <stdio.h>
#include <vector>

#include <stdlib.h>
#include <sys/types.h>
#include <cstring>
#include <iostream>
#include <fstream>

#include <sstream>

#include <sys/stat.h>
#include <unistd.h>
#include <cmath>
#include <set>
#include <stdbool.h>

/*FILE STRUCTURE MACROS*/
#define MAXIMUM_SIZE_OF_FILE 10
#define MAXIMUM_FILES 10240 
/*END FILE STRUCTURE MACROS*/

/*segment structure macros*/
#define NUMBER_OF_SEGMENT 32
#define SIZE_OF_SEGMENT 1048576 
#define BLOCK_PER_SEGMENT 1024 
#define SIZE_OF_BLOCK 1024 
/*end - segment structure macros*/

#define IMAP_BLKS 40 //macro for imap block
#define MAXIMUM_DATA_BLKS 128 //macro for data block
#define BLK_FOR_SUMMARY 8 //macro for summary block in each inode


/*macros for defing clean and dirty segment*/
#define CLEAN_SEG 0 
#define DIRTY_SEG 1
/*end- macros for defing clean and dirty segment*/


#define ASSIGNED_BLOCK 1016 //assigned block is 8 less than total block size
#define FILEMAP_SIZE_OF_BLOCK 256  //size of filemap block

 char NOTVALID[1] = {0};
// tag filenames into the filemap
 char VALID[1] = {127}; 
 unsigned int IMAP[IMAP_BLKS*SIZE_OF_BLOCK]; 
// current segment in memory
 char SEG[SIZE_OF_SEGMENT]; 

 unsigned int BLOCKS_THAT_ARE_AVILABLE = 0; 
 unsigned int SEGMENT_NUMBER = 1; 

 unsigned int CHKPOINT_REG[IMAP_BLKS];
 unsigned int SUMMARY_OF_SEGMENT[BLOCK_PER_SEGMENT][2];


//0 = clean, 1 = not clean
 char NUMBER_OF_CLEAN_SEGMENT[NUMBER_OF_SEGMENT]; 

// structure for inode entry
 typedef struct 
 {
      char name_of_file[FILEMAP_SIZE_OF_BLOCK - 1];
      int size_of_file; 
      int blk_loc[MAXIMUM_DATA_BLKS]; 
 } inode;
 
 // end - structure for inode entry

 std::vector<std::string> split(const std::string &str) 
 {
  std::string b;

  std::stringstream stsr(str);

  std::vector<std::string> t;

  int tok_number = 0;

  while (stsr >> b) t.push_back(b);

  return t;
 }

 //function to find next avilable lfs_filename block in current segment
#pragma region block
 void findadjacentblockavilable()
 {
       bool even_if_one_imap_part = false;
       unsigned int current_imap_position = 0;
       for (int i = 0; i < IMAP_BLKS; ++i)
       {
           if (CHKPOINT_REG[i] != (unsigned int) -1 && CHKPOINT_REG[i] >= current_imap_position)
           {
                current_imap_position = CHKPOINT_REG[i];
                even_if_one_imap_part = true;
           }
       }

       BLOCKS_THAT_ARE_AVILABLE = (even_if_one_imap_part) ? (current_imap_position % SIZE_OF_BLOCK) + 1 : 0;
       SEGMENT_NUMBER = 1 + current_imap_position / SIZE_OF_BLOCK;
  }
#pragma endregion block
//end - function to find next avilable block in current segment

//Function to write checkpoint
#pragma region chkpnt
 void Writtingchkpntregn()
 {
       std::fstream checkpointregion("SegmentChaeckPointDrive/CHECKPOINT_REGION", std::fstream::binary | std::ios::out);

       char buffer[IMAP_BLKS * 4];
       std::memcpy(buffer, CHKPOINT_REG, IMAP_BLKS * 4);
       checkpointregion.write(buffer, IMAP_BLKS * 4);
       checkpointregion.write(NUMBER_OF_CLEAN_SEGMENT, NUMBER_OF_SEGMENT);
       checkpointregion.close();
 }
#pragma endregion chkpnt
//end - Function to write checkpoint

//Function to find next clean segment
#pragma region segcl
 void searchadjacentcleanseg()
 {     
       for (int i = 0; i < NUMBER_OF_SEGMENT; ++i)
       {
          if (NUMBER_OF_CLEAN_SEGMENT[i] == CLEAN_SEG) 
          {
              SEGMENT_NUMBER = i+1;
              return;
            }
       }
       printf("No available memory remaining. Exiting...\n");
       Writtingchkpntregn();
       exit(0);
 }
#pragma endregion segcl
//end - Function to find next clean segment

//Function to write out the segment
#pragma region writeseg
 void writtingoutthesegment()
 {
       std::fstream seg_file("SegmentChaeckPointDrive/SEGMENT"+std::to_string(SEGMENT_NUMBER), std::fstream::binary | std::ios::out);
       seg_file.write(SEG, ASSIGNED_BLOCK * SIZE_OF_BLOCK);
       seg_file.write(reinterpret_cast<const char*>(&SUMMARY_OF_SEGMENT), BLK_FOR_SUMMARY * SIZE_OF_BLOCK);
       seg_file.close();
       searchadjacentcleanseg();
       BLOCKS_THAT_ARE_AVILABLE = 0;
 }

    unsigned int adjacentInodeno()
    {
    std::ifstream filemapping("SegmentChaeckPointDrive/FILEMAP");
     for (int i = 0; i < MAXIMUM_FILES; ++i)
     {
         filemapping.seekg(i*FILEMAP_SIZE_OF_BLOCK);
         char valid[1];
         filemapping.read(valid, 1);
         if (!valid[0]) {
            filemapping.close();
            return i;
         }
      }
      filemapping.close();
      return -1;
    }
#pragma endregion writeseg
//end - Function to write out the segment

//function to read inside the segment
#pragma region readseg
 void readinginsidesegment()
 {
     std::fstream seg_file("SegmentChaeckPointDrive/SEGMENT"+std::to_string(SEGMENT_NUMBER), std::fstream::binary | std::ios::in);
     seg_file.read(SEG, ASSIGNED_BLOCK * SIZE_OF_BLOCK);
     char buffer[BLK_FOR_SUMMARY * SIZE_OF_BLOCK];
     seg_file.read(buffer, BLK_FOR_SUMMARY * SIZE_OF_BLOCK);
     std::memcpy(&SUMMARY_OF_SEGMENT, buffer, BLK_FOR_SUMMARY * SIZE_OF_BLOCK);
     seg_file.close();
 }
#pragma endregion readseg
//function to read inside the segment

//fuction to read imap block
#pragma region rdimap 
 void Imapblockreading(unsigned int add, unsigned int frag_number)
 {
       unsigned int segment_no = (add / BLOCK_PER_SEGMENT) + 1;
       unsigned int block_start_pos = (add % BLOCK_PER_SEGMENT) * SIZE_OF_BLOCK;
       std::fstream seg_file("SegmentChaeckPointDrive/SEGMENT" + std::to_string(segment_no), std::fstream::binary | std::ios::in | std::ios::out);
       char buffer[SIZE_OF_BLOCK];
       seg_file.seekg(block_start_pos);
       seg_file.read(buffer, SIZE_OF_BLOCK);
       std::memcpy(&IMAP[frag_number*(SIZE_OF_BLOCK/4)], buffer, SIZE_OF_BLOCK);
       seg_file.close();
 }
#pragma endregion rdimap 
//end - fuction to read imap block

// read from checkpointregion
#pragma region rdchkpt
 void CheckpointRegionReading()
 {
     std::fstream checkpointregion("SegmentChaeckPointDrive/CHECKPOINT_REGION", std::ios::binary | std::ios::out | std::ios::in);
     char buffer[IMAP_BLKS * 4];
     checkpointregion.read(buffer, IMAP_BLKS * 4);
     std::memcpy(CHKPOINT_REG, buffer, IMAP_BLKS * 4);
     checkpointregion.read(NUMBER_OF_CLEAN_SEGMENT, NUMBER_OF_SEGMENT);
     checkpointregion.close();
}
#pragma endregion rdchkpt
//End - read from checkpointregion

//fuction to read inside imap
#pragma region rdimap
 void Imapreading()
 {
       for (unsigned int i = 0; i < IMAP_BLKS; ++i)
       {
         if (CHKPOINT_REG[i] != (unsigned int) -1)
            Imapblockreading(CHKPOINT_REG[i], i);
        }
  }
#pragma endregion rdimap
//end - fuction to read inside imap

//function to update imap
#pragma region upimap
 void updatetheimap(unsigned int inode_no, unsigned int blk_postn){
       if (BLOCKS_THAT_ARE_AVILABLE == BLOCK_PER_SEGMENT)
       {
           writtingoutthesegment();
       }  
       IMAP[inode_no] = blk_postn;
       unsigned int frag_number = inode_no / (BLOCK_PER_SEGMENT / 4);
       std::memcpy(&SEG[BLOCKS_THAT_ARE_AVILABLE * SIZE_OF_BLOCK], &IMAP[frag_number * (SIZE_OF_BLOCK / 4)], SIZE_OF_BLOCK);
       SUMMARY_OF_SEGMENT[BLOCKS_THAT_ARE_AVILABLE][0] = -1;
       SUMMARY_OF_SEGMENT[BLOCKS_THAT_ARE_AVILABLE][1] = frag_number;
       CHKPOINT_REG[frag_number] = BLOCKS_THAT_ARE_AVILABLE + (SEGMENT_NUMBER - 1) * BLOCK_PER_SEGMENT;
       NUMBER_OF_CLEAN_SEGMENT[SEGMENT_NUMBER - 1] = DIRTY_SEG;
       BLOCKS_THAT_ARE_AVILABLE++;
 }

 int MeasureFilesize(int inode_no){
       unsigned int blk_postn = IMAP[inode_no];
       unsigned int seg_loc = blk_postn/BLOCK_PER_SEGMENT + 1;
       unsigned int loc_blk_pos = (blk_postn % BLOCK_PER_SEGMENT) * SIZE_OF_BLOCK;
       inode metadata;
       if(SEGMENT_NUMBER == seg_loc)
       {
          std::memcpy(&metadata, &SEG[loc_blk_pos], sizeof(inode));
       }else
       {
          std::fstream segment_on_disc("SegmentChaeckPointDrive/SEGMENT" + std::to_string(seg_loc), std::ios::binary | std::ios::in);
          segment_on_disc.seekg(loc_blk_pos);
          char buffer[sizeof(inode)];
          segment_on_disc.read(buffer, sizeof(inode));
          std::memcpy(&metadata, buffer, sizeof(inode));
          segment_on_disc.close();
        }
       return metadata.size_of_file;
 }

 unsigned int findfileInodeno(std::string lfs_nameoffile)
 {
       std::fstream filemapping("SegmentChaeckPointDrive/FILEMAP", std::ios::binary | std::ios::in | std::ios::out);
       for (unsigned int i = 0; i < MAXIMUM_FILES; ++i)
       {
          filemapping.seekg(i*FILEMAP_SIZE_OF_BLOCK);
          char valid[1];
          filemapping.read(valid, 1);
          if (valid[0]) 
          {
             char nameoffile_buf[FILEMAP_SIZE_OF_BLOCK-1];
             filemapping.read(nameoffile_buf, FILEMAP_SIZE_OF_BLOCK-1);
             std::string nameoffile(nameoffile_buf);
             if (nameoffile == lfs_nameoffile)
             {
                 filemapping.close();
                 return i;
             }
          }
        }
       filemapping.close();
       return (unsigned int) -1;
 }
#pragma endregion upimap
//end- function to update imap

//function to update filemap
#pragma region upflmap
 void filemapupdatting(unsigned int inode_no, std::string lfs_nameoffile)
 {
       std::fstream filemapping("SegmentChaeckPointDrive/FILEMAP", std::fstream::binary | std::ios::out | std::ios::in);
       filemapping.seekp(inode_no * FILEMAP_SIZE_OF_BLOCK);
       filemapping.write(VALID, 1);
       filemapping.write(lfs_nameoffile.c_str(), lfs_nameoffile.length()+1);
       filemapping.close();
 }
#pragma endregion upflmap
//end - function to update filemap

//function to write inode
#pragma region wrinode
 void Inodewrirring(const inode& nde, unsigned int inode_no)
 {
       SUMMARY_OF_SEGMENT[BLOCKS_THAT_ARE_AVILABLE][0] =  inode_no;
       SUMMARY_OF_SEGMENT[BLOCKS_THAT_ARE_AVILABLE][1] = (unsigned int) -1;
       //write that inode to the next BLOCK
       std::memcpy(&SEG[BLOCKS_THAT_ARE_AVILABLE*SIZE_OF_BLOCK], &nde, sizeof(inode));
       BLOCKS_THAT_ARE_AVILABLE++;
 }
#pragma endregion wrinode
//function to write inode

//function to print block
#pragma region print the block
 void printingtheblk(unsigned int global_block_pos, unsigned int start_byte, unsigned int end_byte, bool first_block, bool last_block)
 {
       unsigned int segment_no = (global_block_pos / BLOCK_PER_SEGMENT) + 1;
       unsigned int loc_blk_pos = (global_block_pos % BLOCK_PER_SEGMENT) * SIZE_OF_BLOCK;
       if (first_block)loc_blk_pos += start_byte;
       unsigned int buffer_size;
       if (first_block && last_block)buffer_size = end_byte - start_byte;
       else if (last_block) buffer_size = end_byte % SIZE_OF_BLOCK;
       else if (first_block) buffer_size = SIZE_OF_BLOCK - (start_byte % SIZE_OF_BLOCK);
       else buffer_size = SIZE_OF_BLOCK;
       char buffer[buffer_size];

       if (segment_no != SEGMENT_NUMBER)
       {
          std::fstream seg_file("SegmentChaeckPointDrive/SEGMENT"+std::to_string(segment_no), std::ios::binary | std::ios::in);
          seg_file.seekg(loc_blk_pos);
          seg_file.read(buffer, buffer_size);
          seg_file.close();
       }else
       {
          std::memcpy(buffer, &SEG[loc_blk_pos], buffer_size);
       }

       for (int i = 0; i < buffer_size; ++i)
       {
          printf("%c", buffer[i]);
       }
       if (last_block) {
        printf("\n");
       }
 }

 inode obtaintheInode(unsigned int inode_no)
 {
       unsigned int global_block_pos = IMAP[inode_no];
       unsigned int segment_no = (global_block_pos / BLOCK_PER_SEGMENT) + 1;
       unsigned int loc_blk_pos = (global_block_pos % BLOCK_PER_SEGMENT) * SIZE_OF_BLOCK;
       inode metadata;
       if (global_block_pos == (unsigned int) -1) 
       { // if there isn't an inode to be returned
            metadata.size_of_file = -1;
            return metadata;
       }
       if (segment_no != SEGMENT_NUMBER)
       {
            std::fstream seg_file("SegmentChaeckPointDrive/SEGMENT"+std::to_string(segment_no), std::ios::binary | std::ios::in);
            seg_file.seekg(loc_blk_pos);
            char buffer[SIZE_OF_BLOCK];
            seg_file.read(buffer, SIZE_OF_BLOCK);
            std::memcpy(&metadata, buffer, sizeof(inode));
            seg_file.close();
       }else
       {
            std::memcpy(&metadata, &SEG[loc_blk_pos], sizeof(inode));
       }
       return metadata;
 }
#pragma endregion print the block
//end - function to print block

//function to write number of clean segment
#pragma region writing segment that are clean
 void writtingsegmentthatareclean(unsigned int clean_summary[BLOCK_PER_SEGMENT][2], char clean_segment[ASSIGNED_BLOCK * SIZE_OF_BLOCK], unsigned int& next_available_block_clean, int& clean_segment_no, std::vector<inode>& inodes, std::set<int>& fragments)
 {
      for (int i = 0; i < inodes.size(); ++i) 
       {
          std::memcpy(&clean_segment[next_available_block_clean * SIZE_OF_BLOCK], &inodes[i], sizeof(inode));
          int inode_no = findfileInodeno(inodes[i].name_of_file);
          clean_summary[next_available_block_clean][0] = inode_no;
          clean_summary[next_available_block_clean][1] = -1;
          IMAP[inode_no] = (clean_segment_no - 1) * BLOCK_PER_SEGMENT + next_available_block_clean;
          next_available_block_clean++;
       }
      for (auto frag_number: fragments)
       {
          std::memcpy(&clean_segment[next_available_block_clean * SIZE_OF_BLOCK], &IMAP[frag_number * (SIZE_OF_BLOCK / 4)], SIZE_OF_BLOCK);
          clean_summary[next_available_block_clean][0] = -1;
          clean_summary[next_available_block_clean][1] = frag_number;
          CHKPOINT_REG[frag_number] = next_available_block_clean + (clean_segment_no - 1) * BLOCK_PER_SEGMENT;
          next_available_block_clean++;
       }
       std::fstream seg_file("SegmentChaeckPointDrive/SEGMENT"+std::to_string(clean_segment_no), std::fstream::binary | std::ios::out);
       seg_file.write(clean_segment, ASSIGNED_BLOCK * SIZE_OF_BLOCK);
       char buffer[BLK_FOR_SUMMARY * SIZE_OF_BLOCK];
       std::memcpy(buffer, clean_summary, BLK_FOR_SUMMARY * SIZE_OF_BLOCK);
       seg_file.write(buffer, BLK_FOR_SUMMARY * SIZE_OF_BLOCK);
       seg_file.close();
       fragments.clear();
       inodes.clear();
       NUMBER_OF_CLEAN_SEGMENT[clean_segment_no - 1] = DIRTY_SEG;
       next_available_block_clean = 0;
       for (int i = 0; i < BLOCK_PER_SEGMENT; ++i) 
       {
          for (int j = 0; j < 2; ++j)
          {
            clean_summary[i][j] = (unsigned int) -1;
          }
       }
       clean_segment_no++;
 }
#pragma endregion writing segment that are clean
//end - function to write number of clean segment

//function to clean dirty segment
#pragma region dirty segment cleaning
 void cleaningthesegment(int dirty_segment_no, unsigned int clean_summary[BLOCK_PER_SEGMENT][2], char clean_segment[ASSIGNED_BLOCK * SIZE_OF_BLOCK], unsigned int& next_available_block_clean, int& clean_segment_no, std::vector<inode>& inodes, std::set<int>& fragments)
 {
   // import dirty segment into memory
       unsigned int dirty_summary[BLOCK_PER_SEGMENT][2];
       char dirty_segment[ASSIGNED_BLOCK * SIZE_OF_BLOCK];
       if (dirty_segment_no == SEGMENT_NUMBER)
       {
            std::memcpy(dirty_summary, SUMMARY_OF_SEGMENT, BLK_FOR_SUMMARY * SIZE_OF_BLOCK);
            std::memcpy(dirty_segment, SEG, ASSIGNED_BLOCK * SIZE_OF_BLOCK);
       }else
       {
          char summary_buffer[BLK_FOR_SUMMARY * SIZE_OF_BLOCK];
          std::fstream seg_file("SegmentChaeckPointDrive/SEGMENT"+std::to_string(dirty_segment_no), std::ios::in | std::ios::out | std::ios::binary);
          seg_file.read(dirty_segment, ASSIGNED_BLOCK * SIZE_OF_BLOCK);
          seg_file.read(summary_buffer, BLK_FOR_SUMMARY * SIZE_OF_BLOCK);
          std::memcpy(dirty_summary, summary_buffer, BLK_FOR_SUMMARY * SIZE_OF_BLOCK);
          seg_file.close();
       }
       for (int i = 0; i < ASSIGNED_BLOCK; ++i)
       {
            unsigned int inode_no = dirty_summary[i][0];
            unsigned int block_no = dirty_summary[i][1];

            if (inode_no != (unsigned int) -1 && block_no != (unsigned int) -1)//--------datablock--------
            {       
              if (ASSIGNED_BLOCK - next_available_block_clean < 3 + fragments.size() + inodes.size())
                  writtingsegmentthatareclean(clean_summary, clean_segment, next_available_block_clean, clean_segment_no, inodes, fragments);
                  inode old_node = obtaintheInode(inode_no);
              if (old_node.size_of_file != (unsigned int) -1 && old_node.blk_loc[block_no] == (dirty_segment_no-1) * BLOCK_PER_SEGMENT + i) //if data block is live
              { //if data block is live        //deal with imap piece
                    fragments.insert(inode_no / (BLOCK_PER_SEGMENT / 4));
                //deal with inode
                    bool duplicate_inode = false; // sees whether this inode was already in our vector
                    for (int j = 0; j < inodes.size(); ++j) 
                    {
                      if (strcmp(inodes[j].name_of_file, old_node.name_of_file) == 0)
                      {
                        inodes[j].blk_loc[block_no] = (clean_segment_no-1) * BLOCK_PER_SEGMENT + next_available_block_clean;
                        duplicate_inode = true;
                        break;
                      }
                    }
                    if (!duplicate_inode)
                    {
                      old_node.blk_loc[block_no] = (clean_segment_no-1) * BLOCK_PER_SEGMENT + next_available_block_clean;
                      inodes.push_back(old_node);
                    }
            //here dealing with real data
                    std::memcpy(&clean_segment[next_available_block_clean * SIZE_OF_BLOCK], &dirty_segment[i * SIZE_OF_BLOCK], SIZE_OF_BLOCK);
                    clean_summary[next_available_block_clean][0] = inode_no;
                    clean_summary[next_available_block_clean][1] = block_no;
                    next_available_block_clean++;
              }
           }else if (inode_no != (unsigned int) -1 && block_no == (unsigned int) -1) //--------inode block--------
           { 
              if (ASSIGNED_BLOCK - next_available_block_clean < 2 + fragments.size() + inodes.size())
                  writtingsegmentthatareclean(clean_summary, clean_segment, next_available_block_clean, clean_segment_no, inodes, fragments);
              if (IMAP[inode_no] == (dirty_segment_no-1) * BLOCK_PER_SEGMENT + i) 
              { //if actual inode
                  inode old_node = obtaintheInode(inode_no);
                  fragments.insert(inode_no / (BLOCK_PER_SEGMENT / 4));
                  bool duplicate_inode = false; // sees whether this inode was already in our vector
                  for (int j = 0; j < inodes.size(); ++j) 
                  {
                      if (strcmp(inodes[j].name_of_file, old_node.name_of_file) == 0)
                      {
                          duplicate_inode = true;
                          break;
                      }
                  }
                  if (!duplicate_inode)
                      inodes.push_back(old_node);
              }
            }else if (inode_no == (unsigned int) -1 && block_no != (unsigned int) -1)
            { //--------imap fragment--------
              if (ASSIGNED_BLOCK - next_available_block_clean < 1 + fragments.size() + inodes.size())
                writtingsegmentthatareclean(clean_summary, clean_segment, next_available_block_clean, clean_segment_no, inodes, fragments);
                fragments.insert(block_no);
            }
      }
 }
#pragma endregion dirty segment cleaning
//function to clean dirty segment


