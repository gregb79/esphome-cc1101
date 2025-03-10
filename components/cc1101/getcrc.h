std::vector<int> getcrc() {
    
       uint8_t data[12] = {0xAA, 0XAA, 0XAA, 0XAA, 0x2D, 0xD4, 0xF9, 203 , 0x00, 17, 3 , 0x00};
    uint8_t datacrc[12] = {0x00, 0X00, 0X00, 0X00, 0x00, 0x00, 0x00, 0x00 , 0x00, 0x00, 0x00 , 0x00};
       //uint8_t data[] = {0x00, 0X00, 0X00, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
       //                    0     1     2     3     4     5     6     7     8     9     10    11                     
       //                                            |__CRC =__0x100_-_sum_of_bytes______|     CRC
       //                                                              ID         INST   MODE

        data[6] =  id(SPA_ELECTRIC_ID0_Input).state;   // ID
        data[7] =  id(SPA_ELECTRIC_ID1_Input).state;   // ID
        data[9] =  id(SPA_ELECTRIC_INSTRUCTION_Input).state;   // INSTRUCTION
//        register_value_1->publish_state(data[9]);

        
        if(id(SPA_ELECTRIC_MODE_Select).state == "Pool Only"){
        data[10] =  1;   // MODE
//        id(SPA_ELECTRIC_MODE_Input).state = data[10];
        }
        else if (id(SPA_ELECTRIC_MODE_Select).state == "Spa Only"){
        data[10] =  2;   // MODE
//        id(SPA_ELECTRIC_MODE_Input).state = data[10];
        }
        else if (id(SPA_ELECTRIC_MODE_Select).state == "Pool and Spa"){
        data[10] =  3;   // MODE
//        id(SPA_ELECTRIC_MODE_Input).state = data[10];
        }
//       data[10] =  id(SPA_ELECTRIC_MODE_Input).state;   // MODE

       unsigned char checksum = calculateChecksum(data);

        Serial.print("Checksum = : ");
        Serial.println(checksum,HEX);
        data[11] = checksum;

     
     //   Serial.println();
        // copy data to datacrc
        int h;
        for( int h=0; h<12; h++)
        {
        datacrc[h] = data[h];
        }
      
      //// Prints Hex output
      Serial.println();
      Serial.print("HEX output : ");
      int j = 0;
      for (int j=0; j<12; j++) {
      Serial.printf("%02x", datacrc[j], HEX);
      }  
      Serial.println();



    std::vector<int> DataVector;

    for (int i = 0; i < 12; ++i) {
        uint8_t byte = datacrc[i];
        for (int j = 7; j >= 0; --j) {
            if (byte & (1 << j)) {
                DataVector.push_back(105);  // High bit (1) corresponds to 105 microseconds
            } else {
                DataVector.push_back(-104); // Low bit (0) corresponds to -104 microseconds
            }
        }
    }
    
    // Convert DataVector elements to a single string
    std::string dataString;
    for (int value : DataVector) {
        dataString += std::to_string(value) + ", ";
    }

    // Remove the trailing comma and space
    if (!dataString.empty()) {
        dataString.erase(dataString.length() - 2);
    }
    // Log the string
    ESP_LOGD("custom", "New Vector: %s", dataString.c_str());

    return DataVector;
}

public:
    //unsigned char calculateChecksum(const uint8_t *data, size_t length) {
    unsigned char calculateChecksum(const uint8_t *data){ 
    unsigned int sum = 0;
    Serial.print("Calculate Checksum From : ");
    // Calculate the sum of bytes
  
    //for (size_t i = 0; i < length; ++i) {
    int i = 0;
    for (int i=4; i<11; i++) {

      Serial.printf("%02x", data[i], HEX);
      Serial.print(" ");
      sum += data[i];
    }  

   // Take the two's complement and extract the lower 8 bits
    unsigned char checksum = static_cast<unsigned char>(-sum);

    return checksum;
    Serial.println();
    Serial.println();
  }

};

