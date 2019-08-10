#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <filesystem>

void get_string_positions( std::vector<size_t>& out, std::string input, std::string target )
{
	std::transform( input.begin(), input.end(), input.begin(), ::tolower );
	std::transform( target.begin(), target.end(), target.begin(), ::tolower );

	size_t pos = input.find( target );
	while( pos != std::string::npos )
	{
		out.push_back( pos );

		pos = input.find( target, pos + target.size() );
	}
}

int main()
{
	std::string appdata( getenv( "APPDATA" ) );
	appdata += "\\Discord\\Local Storage\\leveldb";
	printf( "%s\n\n", appdata.c_str() );

	std::vector< std::experimental::filesystem::v1::path > ldb_entries;
	
	for( const auto& entry : std::experimental::filesystem::directory_iterator( appdata ) )
	{
		if( entry.path().string().find( ".ldb" ) != std::string::npos )
		{
			std::cout << entry.path() << std::endl;
			ldb_entries.push_back( entry );
		}
	}

	printf( "\n" );
	
	// Discord token starts with M, m, N or n
	std::vector< std::string > starting_chars = { "\"M", "\"N" };

	for( std::size_t i = 0; i < ldb_entries.size(); i++ )
	{
		auto ldb_file_path = ldb_entries.at( i ).string();

		auto ldb_file_short = ldb_entries.at( i ).filename();

		std::ifstream f( ldb_file_path, std::ios::binary );
		std::string file_text( ( std::istreambuf_iterator<char>( f ) ), std::istreambuf_iterator<char>() );

		if( f.bad() )
			continue;

		for( auto starting_char : starting_chars )
		{
			printf( "[%ws] Searching for %s\" Token\n", ldb_file_short.c_str(), starting_char.c_str() );

			auto start_time = std::chrono::high_resolution_clock::now();
			
			// Get starting position of string that matches 
			std::vector<size_t> positions;
			get_string_positions( positions, file_text, starting_char ); // I put everything to lower case here, so i dont need to scan each file 4 times but only two (:-D)

			std::vector<std::string> tokens;

			for( auto pos : positions )
			{
				/*
				The string will be treated as token if
				- It starts with starting_char
				- It contains atleast one of these: ., _, -,
				- It doesn't contain ',', {, }, <, >, /
				- The last character of the string is "
				*/
				for( int i = 48; i < 128; i++ )
				{
					std::string file = file_text;

					if( pos + i < file_text.size() )
					{
						auto possible_token = file.substr( pos, i );
						if( ( possible_token.find( "." ) != std::string::npos ||
							possible_token.find( "_" ) != std::string::npos ||
							possible_token.find( "-" ) != std::string::npos ) &&
							possible_token.find( "," ) == std::string::npos &&
							possible_token.find( "{" ) == std::string::npos &&
							possible_token.find( "}" ) == std::string::npos &&
							possible_token.find( ">" ) == std::string::npos &&
							possible_token.find( "<" ) == std::string::npos &&
							possible_token.find( "/" ) == std::string::npos &&
							possible_token.back() == '\"' )
						{
							// Check if string isn't already in array
							bool match = false;
							for( auto token : tokens )
								if( possible_token.find( token ) != std::string::npos )
									match = true;

							if( !match )
								tokens.push_back( possible_token );
						}
					}
				}
			}

			auto end_time = std::chrono::high_resolution_clock::now();

			auto duration = std::chrono::duration_cast< std::chrono::microseconds >( end_time - start_time ).count();

			printf( "[%ws] Searching for %s\" Token Finished in %i microseconds.\n\n", ldb_file_short.c_str(), starting_char.c_str(), static_cast< int >( duration ) );

			if( !tokens.empty() )
			{
				printf( "[%ws] %s\" Tokens found\n", ldb_file_short.c_str(), starting_char.c_str() );
				for( auto token : tokens )
					printf( "[%ws] %s \n", ldb_file_short.c_str(), token.c_str() );

				printf( "\n" );
			}

			tokens.clear();
		}
	}

	system( "PAUSE" );

	return 0;
}
