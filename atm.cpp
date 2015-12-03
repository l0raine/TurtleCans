/*******************************************************************************
Name: Samuel Wenninger, Brandon Drumheller, and Malcolm Lorber                          
Date Created: 11-25-2015                                                                
Filename: proxy.cpp                                                                     
Description: ATM program that is used to communicate with the Bank securely (see
             README for more information). Compile using the following command:             
             g++ -std=c++11 atm.cpp -lboost_system -lboost_thread -lboost_regex
             -pthread -o atm.out 
*******************************************************************************/

//Standard library
#include <iostream>
#include <string>
#include <fstream>
//Boost
#include <boost/asio.hpp>
//Contains all required input validation checks
#include "validate.h"

using boost::asio::ip::tcp;

/*******************************************************************************
 @DESC: Establish connection to the Bank, send the user inputted commands to the
        Bank (if valid), receive the response from the Bank, and display the
        response to the user. The user is able to continually enter commands,
        even after entering an invalid command.
 @ARGS: Port number used to connect to the proxy.
 @RTN:  EXIT_SUCCESS on success, EXIT_FAILURE on failure.
*******************************************************************************/
int main(int argc, char** argv) {
    try {
        if (argc != 2) {
            std::cerr << "ERROR: USAGE <port-to-proxy>" << std::endl;
            return EXIT_FAILURE;
        }
        const std::string ProxyPort = argv[1]; 
        const std::string host = "127.0.0.1";
        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), host, ProxyPort);
        tcp::resolver::iterator iterator = resolver.resolve(query);
        tcp::socket s(io_service);
        boost::asio::connect(s, iterator);        
        std::string request;
	while (std::getline(std::cin, request)) {
            //Validate the user inputted command using regex (see "validate.h")
            if (!IsValidATMCommand(request)) {
                std::cerr << "INVALID COMMAND" << std::endl;
                continue;
            }
            //Setup login using ATM cards.
            if (request.find("login") == 0){
                int space = request.find(" ") +1;
                int account = 0;
                std::string accountStr = "";
                std::string user = request.substr(space, request.size() - space);
                std::ifstream cardFile(user + ".card");
                if(cardFile.is_open()){
                    cardFile >> account;
                    std::stringstream ss;
                    ss << account;
                    accountStr = ss.str();
                }
                else{
                    std::cerr << "Unable to read card file" << std::endl;
                } 
                cardFile.close();               
                //Replace "login <username>" with "login <account-num>" before
                //sending to the Bank.
                request = "login " + accountStr;
            }
            boost::system::error_code EC;
            boost::asio::write(s, boost::asio::buffer(request),
                                            boost::asio::transfer_all(), EC);
            if ((boost::asio::error::eof == EC) ||                           
                              (boost::asio::error::connection_reset == EC)) {
                std::cerr << "ERROR" << std::endl;
            }
            //Read and display the Bank's response.
            boost::asio::streambuf response;
            boost::asio::read_until(s, response, "\0");
            std::istream response_stream(&response);
            std::string answer;
            std::getline(response_stream, answer);
            std::cout << answer << std::endl;
        }
    }
    //If anything goes wrong, the Bank will terminate its connection with the
    //ATM.
    catch (std::exception & e){
        std::cerr << "DISCONNECTED FROM BANK" << std::endl;
    }
    return EXIT_SUCCESS;
}
