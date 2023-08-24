#include "func.cpp"

#pragma region Remove
//Remove the file from file system
 void deletefile(std::string lfs_nameoffile) 
 {
     unsigned int inode_no = findfileInodeno(lfs_nameoffile);
     if (inode_no == (unsigned int) -1){
        std::cout << "File not found." << std::endl;
        return;
     }
     std::fstream filemapping("SegmentChaeckPointDrive/FILEMAP", std::ios::binary | std::ios::in | std::ios::out);
     filemapping.seekp(inode_no*FILEMAP_SIZE_OF_BLOCK);
     filemapping.write(NOTVALID, 1);
     filemapping.close();
     updatetheimap(inode_no, (unsigned int) -1);
 }
////End: Remove the file from file system
#pragma endregion Remove



#pragma region Import
  //import the file 
 void import(std::string nameoffile, std::string lfs_nameoffile) 
 {
       std::ifstream in(nameoffile);
       if (!in.good())
         {
             std::cout << "Could not find file." << std::endl;
             return;
         }
  //End - import the file

  //get the length of the input file
       in.seekg(0, std::ios::end);
       int inputfilesize = in.tellg();
       in.seekg(0, std::ios::beg);
  //end get the length of the input file
        if ((inputfilesize / SIZE_OF_BLOCK) + 3 > ASSIGNED_BLOCK - BLOCKS_THAT_ARE_AVILABLE)
                   writtingoutthesegment();

//check the length of file name
        if (lfs_nameoffile.length() > 254)
           {
              std::cout << "Name of file is to large." << std::endl;
              in.close();
              return;
           }
//get the inode number 
        if (findfileInodeno(lfs_nameoffile) != (unsigned int) -1)
           {
               std::cout << "Name of file already exist." << std::endl;
               in.close();
               return;
          }

        unsigned int inode_no = adjacentInodeno();
  //check if the maxium file import is reached
        if (inode_no == -1) 
            {
               std::cout << "maxiumum number of file reached." << std::endl;
               in.close();
               return;
            }
 //file mapping is updated
        filemapupdatting(inode_no, lfs_nameoffile);

  //After reading from the import file write the file content in segment blocks
        char buffer[inputfilesize];
        in.read(buffer, inputfilesize);
        std::memcpy(&SEG[BLOCKS_THAT_ARE_AVILABLE * SIZE_OF_BLOCK], buffer, inputfilesize);

  //Inode Blocks for metadata
        inode metadata;
        for (int i = 0; i < lfs_nameoffile.length(); ++i)
        {
               metadata.name_of_file[i] = lfs_nameoffile[i];
        }
        metadata.name_of_file[lfs_nameoffile.length()] = '\0';
        metadata.size_of_file = inputfilesize;
        for (unsigned int i = 0; i <= inputfilesize/SIZE_OF_BLOCK; ++i){
        metadata.blk_loc[i] = BLOCKS_THAT_ARE_AVILABLE + (SEGMENT_NUMBER-1)*BLOCK_PER_SEGMENT;
       SUMMARY_OF_SEGMENT[BLOCKS_THAT_ARE_AVILABLE][0] = inode_no;
       SUMMARY_OF_SEGMENT[BLOCKS_THAT_ARE_AVILABLE][1] = i;
       BLOCKS_THAT_ARE_AVILABLE++;
  }

        Inodewrirring(metadata, inode_no);
  ////End - Inode Blocks for metadata
  //Update imap and checkpoint Region
         updatetheimap(inode_no, (BLOCKS_THAT_ARE_AVILABLE - 1) + (SEGMENT_NUMBER-1)*BLOCK_PER_SEGMENT);
         in.close();
 }
#pragma endregion Import

#pragma region Display

//diplay the bytes of the file that user request to dispaly
 void display(std::string lfs_nameoffile, std::string count, std::string start) 
 {
      unsigned int inode_no = findfileInodeno(lfs_nameoffile);

      if (inode_no == (unsigned int) -1)
      {
         std::cout << "File not found." << std::endl;
         return;
      }

      inode metadata = obtaintheInode(inode_no);

      int firstbyte = std::stoi(start);
      int lastbyte= std::stoi(count) + firstbyte;

      if (firstbyte > metadata.size_of_file) 
      {
          printf("Cannot specify a start byte greater than filesize.\n");
          return;
      }

      if (lastbyte> metadata.size_of_file) 
      {
          printf("\t\t  ALERT \n\tAmount specified too great, displaying until end of file.\n");
          lastbyte= metadata.size_of_file;
      }

      for (int i = firstbyte/SIZE_OF_BLOCK; i <= lastbyte/SIZE_OF_BLOCK; ++i)
          printingtheblk(metadata.blk_loc[i], firstbyte, lastbyte, (i == firstbyte/SIZE_OF_BLOCK), (i == lastbyte/SIZE_OF_BLOCK));
 }
// End Display 
#pragma endregion Display

#pragma region ReadData
//Read the content of the file
 void containoffile(std::string lfs_nameoffile) 
 {
     unsigned int inode_no = findfileInodeno(lfs_nameoffile);

     if (inode_no == (unsigned int) -1)
      {
        std::cout << "File not found." << std::endl;
        return;
      }
     inode metadata = obtaintheInode(inode_no);
     int no_data_blocks = (metadata.size_of_file / SIZE_OF_BLOCK) + 1;
     for (int i = 0; i < no_data_blocks; ++i)
         printingtheblk(metadata.blk_loc[i], 0, metadata.size_of_file, (i == 0), (i == no_data_blocks - 1));
 }
//End Reading the content
#pragma endregion ReadData


#pragma region List
 void list() 
 {
     std::cout << "Active Segment: " << SEGMENT_NUMBER << std::endl;
     std::ifstream filemapping("SegmentChaeckPointDrive/FILEMAP");
     for (int i = 0; i < MAXIMUM_FILES; ++i)
       {
           filemapping.seekg(i*FILEMAP_SIZE_OF_BLOCK);
           char valid[1];
           filemapping.read(valid, 1);
           if (valid[0]) 
            {
                char nameoffile[FILEMAP_SIZE_OF_BLOCK-1];
                filemapping.read(nameoffile, FILEMAP_SIZE_OF_BLOCK-1);
                 printf("%s %d\n", nameoffile, MeasureFilesize(i));
            }
       }
        filemapping.close();
 }
#pragma endregion List

#pragma region change
//Overwrite the bytes in the file system
 void change(std::string lfs_nameoffile, std::string amt_stri, std::string start, std::string character_string) 
 {
      int count = std::stoi(amt_stri);
      int firstbyte = std::stoi(start);
      int lastbyte= count + firstbyte;
      char character = character_string.c_str()[0];
      unsigned int inode_no = findfileInodeno(lfs_nameoffile);
      if (inode_no == (unsigned int) -1)
        {
           std::cout << "File not found." << std::endl;
           return;
        }
      inode metadata = obtaintheInode(inode_no);
      if (firstbyte > metadata.size_of_file) 
        {
           printf("you have specify a start byte which is greater than filesize.\n");
           return;
        }
      if(lastbyte> MAXIMUM_DATA_BLKS*SIZE_OF_BLOCK)
        {
           printf("you have specify a start byte which is greater than filesize.\n");
           return;
        }
      int beginingblock = firstbyte / SIZE_OF_BLOCK;
      int lastblock = lastbyte/ SIZE_OF_BLOCK;
      int blocksthatareneeded = lastblock - beginingblock + 3;
      if (blocksthatareneeded > ASSIGNED_BLOCK - BLOCKS_THAT_ARE_AVILABLE)
          writtingoutthesegment();
      char charaterthatarerepeated[count];
      for (int i = 0; i < count; ++i)
          charaterthatarerepeated[i] = character;
      int beginingblock_seg = (metadata.blk_loc[beginingblock] / BLOCK_PER_SEGMENT) + 1;
      if (beginingblock_seg != SEGMENT_NUMBER)
       {
           char buffer[firstbyte % SIZE_OF_BLOCK];
           std::fstream beginingblock_seg_file("SegmentChaeckPointDrive/SEGMENT"+std::to_string(beginingblock_seg), std::ios::in | std::ios::out | std::ios::binary);
           beginingblock_seg_file.seekg((metadata.blk_loc[beginingblock] % BLOCK_PER_SEGMENT) * SIZE_OF_BLOCK);
           beginingblock_seg_file.read(buffer, firstbyte % SIZE_OF_BLOCK);
           beginingblock_seg_file.close();
           std::memcpy(&SEG[BLOCKS_THAT_ARE_AVILABLE * SIZE_OF_BLOCK], buffer, firstbyte % SIZE_OF_BLOCK);
       }
       else
       {
           std::memcpy(&SEG[BLOCKS_THAT_ARE_AVILABLE * SIZE_OF_BLOCK], &SEG[(metadata.blk_loc[beginingblock] % BLOCK_PER_SEGMENT) * SIZE_OF_BLOCK], firstbyte % SIZE_OF_BLOCK);
       }
       std::memcpy(&SEG[BLOCKS_THAT_ARE_AVILABLE * SIZE_OF_BLOCK + firstbyte], charaterthatarerepeated, count);
       if (lastbyte< metadata.size_of_file)
         {
            int endblocksegment = (metadata.blk_loc[lastblock] / BLOCK_PER_SEGMENT) + 1;
            if (endblocksegment != SEGMENT_NUMBER)
                {
                    char buffer[SIZE_OF_BLOCK - lastbyte];
                    std::fstream endblocksegment_file("SegmentChaeckPointDrive/SEGMENT"+std::to_string(endblocksegment), std::ios::in | std::ios::out | std::ios::binary);
                    endblocksegment_file.seekg((metadata.blk_loc[lastblock] % BLOCK_PER_SEGMENT) * SIZE_OF_BLOCK + lastbyte);
                    endblocksegment_file.read(buffer, SIZE_OF_BLOCK - lastbyte);
                    endblocksegment_file.close();
                    std::memcpy(&SEG[BLOCKS_THAT_ARE_AVILABLE * SIZE_OF_BLOCK + lastbyte], buffer, SIZE_OF_BLOCK - lastbyte);
                }
                else
                {
                    std::memcpy(&SEG[BLOCKS_THAT_ARE_AVILABLE * SIZE_OF_BLOCK + lastbyte], &SEG[(metadata.blk_loc[lastblock] % BLOCK_PER_SEGMENT) * SIZE_OF_BLOCK + (lastbyte% SIZE_OF_BLOCK)], SIZE_OF_BLOCK - (lastbyte% SIZE_OF_BLOCK));
                }
          }
          else
          {
             metadata.size_of_file = lastbyte;
          }
       for (int i = beginingblock; i <= lastblock; ++i)
          {
             metadata.blk_loc[i] = BLOCKS_THAT_ARE_AVILABLE + (SEGMENT_NUMBER-1) * BLOCK_PER_SEGMENT;
             SUMMARY_OF_SEGMENT[BLOCKS_THAT_ARE_AVILABLE][0] = inode_no;
             SUMMARY_OF_SEGMENT[BLOCKS_THAT_ARE_AVILABLE][1] = i;
             BLOCKS_THAT_ARE_AVILABLE++;
          }
      Inodewrirring(metadata, inode_no);
      updatetheimap(inode_no, (BLOCKS_THAT_ARE_AVILABLE - 1) + (SEGMENT_NUMBER-1)*BLOCK_PER_SEGMENT);
 }

#pragma region change

#pragma region Exit
 void exit() 
 {
     writtingoutthesegment();
     Writtingchkpntregn();
     exit(0);
 }
#pragma region Exit

#pragma region SegmentClean
 void clean(std::string amt_stri) 
 {
      int count = std::stoi(amt_stri);
      char segment_that_need_cleaning[count];
      int total_segment_need_cleaning = 0;
      for (int i = 0; i < NUMBER_OF_SEGMENT && total_segment_need_cleaning < count; ++i) 
        {
          if (NUMBER_OF_CLEAN_SEGMENT[i] == DIRTY_SEG)
              segment_that_need_cleaning[total_segment_need_cleaning++] = i+1;
        }
      if (total_segment_need_cleaning == 0)
        {
          printf("no segment is dirty hence cleaning is not needed.\n");
          return;
        } 
        else if (total_segment_need_cleaning < count) 
        {
             printf("count of dirty segment is not enough.\n");
             count = total_segment_need_cleaning;
        }
      unsigned int cleanedsum[BLOCK_PER_SEGMENT][2];
      for (int i = 0; i < BLOCK_PER_SEGMENT; ++i) 
        {
          for (int j = 0; j < 2; ++j)
              cleanedsum[i][j] = (unsigned int) -1;
        }
      char segclean[ASSIGNED_BLOCK * SIZE_OF_BLOCK];
      unsigned int adjacent_avilable_clean_blk = 0;
      int number_of_cln_seg = segment_that_need_cleaning[0];
      std::vector<inode> in;
      std::set<int> fgmt;
      if (count > 1) 
            printf("Cleaning %d segments...\n", count);
      else            
             printf("Cleaning 1 segment...\n");
      for (int i = 0; i < count; i++)
         {
            NUMBER_OF_CLEAN_SEGMENT[segment_that_need_cleaning[i]] = CLEAN_SEG;
            printf("Cleaning Segment %d...\n", segment_that_need_cleaning[i]);
            cleaningthesegment(segment_that_need_cleaning[i], cleanedsum, segclean, adjacent_avilable_clean_blk, number_of_cln_seg, in, fgmt);
         }
         writtingsegmentthatareclean(cleanedsum, segclean, adjacent_avilable_clean_blk, number_of_cln_seg, in, fgmt);
         findadjacentblockavilable();
         readinginsidesegment();
 }
#pragma endregion SegmentClean



