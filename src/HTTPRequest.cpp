/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 14:58:18 by migumore          #+#    #+#             */
/*   Updated: 2025/02/28 19:49:59 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HTTPRequest.hpp>

HTTPRequest::HTTPRequest(void){};

HTTPRequest::HTTPRequest(HTTPRequest const &src)
{
	(void)src;
};

HTTPRequest HTTPRequest::operator=(HTTPRequest const &rhs)
{
	(void)rhs;
	return (*this);
};

HTTPRequest::~HTTPRequest(void){};

void HTTPRequest::parser(const std::string &rawRequest)
{
	std::istringstream	requestStream(rawRequest);
	std::string			line;

	if (std::getline(requestStream, line))
	{
		std::istringstream lineStream(line);
		lineStream >> request.method >> request.uri >> request.httpVersion;
	}
	while (std::getline(requestStream, line) && line != "\r")
	{
		std::size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
			std::string key = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 1);
			while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
				value.erase(0, 1);
			request.headers[key] = value;
		}
	}
	if (request.headers.find("Content-Length") != request.headers.end())
	{
		std::string body;
		std::getline(requestStream, body, '\0');
		request.body = body;
	}
}

std::string HTTPRequest::resolveFilePath(const ServerConfig &config) const
{
	std::string matchedLocation;
	std::string resolvedRoot = config.getRoot();
	std::vector<std::string> resolvedIndexFiles = config.getIndexFiles();
	const std::map<std::string, ServerConfigLocation> &locations = config.getLocations();

	for (std::map<std::string, ServerConfigLocation>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		if (request.uri.find(it->first) == 0 && (matchedLocation.empty() || it->first.length() > matchedLocation.length()))
		{
			matchedLocation = it->first;
			resolvedRoot = it->second.getRoot();
			resolvedIndexFiles = it->second.getIndexFiles();
		}
	}

	if (matchedLocation.empty())
	{
		resolvedRoot = config.getRoot();
		resolvedIndexFiles = config.getIndexFiles();
	}

	std::string requestUri = request.uri;
	if (!requestUri.empty() && requestUri[0] == '/')
		requestUri = requestUri.substr(1);

	std::string filePath = resolvedRoot;
	if (!resolvedRoot.empty() && resolvedRoot[resolvedRoot.size() - 1] != '/')
		filePath += "/";
	filePath += requestUri;

	struct stat pathStat;
	if (stat(filePath.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode))
	{
		for (size_t i = 0; i < resolvedIndexFiles.size(); ++i)
		{
			std::string indexPath = filePath;
			if (filePath[filePath.size() - 1] != '/')
				indexPath += "/";
			indexPath += resolvedIndexFiles[i];
			std::ifstream file(indexPath.c_str());
			if (file.good())
				return (indexPath);
		}
	}

	if (!filePath.empty() && filePath[filePath.size() - 1] == '/')
		filePath = filePath.substr(0, filePath.size() - 1);
	Utilities utils;
	std::string fileExtension;
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos != std::string::npos) {
		fileExtension = filePath.substr(dotPos);
		std::string mimeType = utils.getMimeType(fileExtension);
		if (!mimeType.empty())
			return (filePath);
	}

	return (filePath);
}