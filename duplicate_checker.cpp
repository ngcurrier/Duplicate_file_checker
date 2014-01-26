#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
//#include <unistd.h>
#include <dirent.h>

#include "md5wrapper.h"

using namespace std;

struct FileData{
  string filename;
  unsigned int filesize;
  string crc;
  bool sum_calced;
  bool deleted;
};

int GetDirFiles(string dir, vector<FileData> &data);
int MD5(int index, vector<FileData>& data);

int main(int argc, char* argv[]){

  int i, j;
  string directory, directory2;
  string ui_data;
  string full_path;
  vector<FileData> vec;
  int full_matches = 0;
  int files_deleted = 0;
  int files_searched;
  int valid_response;

  if(argc != 2){
    cout << "Parses directory recursively for duplicate files" << endl;
    cout << "based on file size and MD5 checksum" << endl;
    cout << "USAGE: " << argv[0] << " 'directory to parse'" << endl;
    return (1);
  }
  directory = argv[1];
  //directory2 = argv[2];

  //get files in the directory and load their names/sizes into our structure
  GetDirFiles(directory, vec);
  //GetDirFiles(directory2, vec); 

  #if 0
  for(unsigned int i = 0; i < vec.size(); i++){
    cout << vec[i].filename << "\t" << vec[i].filesize << endl;
  }
  #endif
  
  cout << "Searching directory for identical files..." << endl;
  files_searched = vec.size();
  for(i = 0; i < vec.size()-1; i++){
    for(j = i+1; j < vec.size(); j++){
      if(vec[i].filesize == vec[j].filesize && !vec[i].deleted && !vec[j].deleted){
	cout << "ATTN: Size match! " << endl
	     << "1) " << vec[i].filename << endl 
	     << "2) " << vec[j].filename << endl;
	cout << "Calculating checksums..." << endl;
	MD5(i, vec);
	MD5(j, vec);
	if(vec[i].crc == vec[j].crc){
	  full_matches++;
	  cout << vec[i].crc << endl;
	  cout << vec[j].crc << endl;
	  cout << "ATTN: Checsksums also match, likely the same file!" << endl;
	  cout << "Remove file? Please enter 1, 2, or no" << endl;
	  //jump for user input check
	  do{
	    cin >> ui_data;
	    if(ui_data[0] == '1'){
	      cout << "\nRemoving " << vec[i].filename << endl;
	      if( remove( vec[i].filename.c_str() ) != 0 ){
		cerr << "Error deleting file" << endl;
	      }
	      else{
		cout << "File successfully deleted" << endl;
		files_deleted++;
	      }
	      //mark entry deleted from filelist as well
	      //if we actually deleted it from the list
	      //our counters i, j could and will be messed up
	      vec[i].deleted = 1;
	      valid_response = 1;
	    }
	    else if(ui_data[0] == '2'){
	      cout << "\nRemoving " << vec[j].filename << endl;
	      if( remove( vec[j].filename.c_str() ) != 0 ){
		cerr << "Error deleting file" << endl;
	      }
	      else{
		cout << "File successfully deleted" << endl;
		files_deleted++;
	      }
	      //mark entry deleted from filelist as well
	      //if we actually deleted it from the list
	      //our counters i, j could and will be messed up
	      vec[j].deleted = 1;
	      valid_response = 1;
	    }
	    else if(ui_data[0] == 'n'){
	      cout << "\nLeaving " << vec[j].filename << endl;
	      valid_response = 1;
	    }
	    else{
	      cout << "Please enter 1, 2, or no" << endl;
	      //cout << ui_data << endl;
	      valid_response = 0;
	    }
	    
	  }while(valid_response == 0);
	}
	else{
	  cout << "Checksums don't match: false alarm" << endl;
	}
      }
    }
  }

  cout << "Files searched: " << files_searched << endl;
  cout << "Full matches found: " << full_matches << endl;
  cout << "Files deleted: " << files_deleted << endl;

  return (0);

}


int GetDirFiles(string dir, vector<FileData> &data)
{
  DIR *dp;
  struct dirent *dirp;
  string filename;
  FileData temp;
  string crc = "";
  struct stat stat_buffer;
  int err = 0;

  //crc will be calculated later --- set to zero
  temp.crc = crc;
  temp.sum_calced = 0;
  temp.deleted = 0;
 
  if((dp = opendir(dir.c_str())) == NULL){
    cout << "Error opening " << dir << endl;
    return (1);
  }

  while ((dirp = readdir(dp)) != NULL){
    filename = string(dirp->d_name);
    //ignore hidden files
    if(filename[0] == '.') continue;
    //build up the path for stat() to work on
    filename = dir + filename;
    temp.filename = filename;
    err = stat(filename.c_str() , &stat_buffer); 
    if( err != 0){
      cerr << "stat() error... " << filename << endl;
      return (1);
    }
    //make sure we are only parsing files
    if((stat_buffer.st_mode & S_IFMT) == S_IFREG){
      temp.filesize = (unsigned int)stat_buffer.st_size;
      //push the new record onto our vector
      data.push_back(temp);
    }
    else if((stat_buffer.st_mode & S_IFMT) == S_IFDIR){
      //If this is a directory let's recurse into it
      //cout << "Recursing " << filename << endl;
      GetDirFiles(filename + '/', data);
    }
    else continue;
  }


  closedir(dp);

  return (0);
}



int MD5(int index, vector<FileData> &data)
{
  // creating a wrapper object
  md5wrapper md5;
  
  // create a hash from a string
  //string hash1 = md5.getHashFromString("Hello World");
  if(data[index].sum_calced == 0){
    // create a hash from a file
    string hash2 = md5.getHashFromFile(data[index].filename);
    
    data[index].crc = hash2;
    data[index].sum_calced = 1;
  }
  
  return 0;
}
