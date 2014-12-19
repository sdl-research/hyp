




















namespace Util {





class MemoryInfo {
 public:

  // Singleton
  MemoryInfo();
  MemoryInfo(const MemoryInfo&);




  std::size_t getSize(); // TODO: change type?
  double getSizeInMB();
  double getSizeInGB();

 private:



  std::string getColumn(const std::string& s, unsigned columnNumber);

};



#endif
