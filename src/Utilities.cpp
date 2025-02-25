/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utilities.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 18:23:20 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/21 18:37:56 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utilities.hpp"

Utilities::Utilities(void){}

Utilities::~Utilities(void){}

std::string Utilities::getMimeType(const std::string &extension) const
{
	static std::map<std::string, std::string> mimeTypes;
	if (mimeTypes.empty())
	{
		mimeTypes[".html"] = "text/html";
		mimeTypes[".htm"] = "text/html";
		mimeTypes[".shtml"] = "text/html";
		mimeTypes[".css"] = "text/css";
		mimeTypes[".xml"] = "text/xml";
		mimeTypes[".gif"] = "image/gif";
		mimeTypes[".jpeg"] = "image/jpeg";
		mimeTypes[".jpg"] = "image/jpeg";
		mimeTypes[".js"] = "application/x-javascript";
		mimeTypes[".atom"] = "application/atom+xml";
		mimeTypes[".rss"] = "application/rss+xml";
		mimeTypes[".mml"] = "text/mathml";
		mimeTypes[".txt"] = "text/plain";
		mimeTypes[".jad"] = "text/vnd.sun.j2me.app-descriptor";
		mimeTypes[".wml"] = "text/vnd.wap.wml";
		mimeTypes[".htc"] = "text/x-component";
		mimeTypes[".png"] = "image/png";
		mimeTypes[".tif"] = "image/tiff";
		mimeTypes[".tiff"] = "image/tiff";
		mimeTypes[".wbmp"] = "image/vnd.wap.wbmp";
		mimeTypes[".ico"] = "image/x-icon";
		mimeTypes[".jng"] = "image/x-jng";
		mimeTypes[".bmp"] = "image/x-ms-bmp";
		mimeTypes[".svg"] = "image/svg+xml";
		mimeTypes[".svgz"] = "image/svg+xml";
		mimeTypes[".webp"] = "image/webp";
		mimeTypes[".jar"] = "application/java-archive";
		mimeTypes[".war"] = "application/java-archive";
		mimeTypes[".ear"] = "application/java-archive";
		mimeTypes[".hqx"] = "application/mac-binhex40";
		mimeTypes[".doc"] = "application/msword";
		mimeTypes[".pdf"] = "application/pdf";
		mimeTypes[".ps"] = "application/postscript";
		mimeTypes[".eps"] = "application/postscript";
		mimeTypes[".ai"] = "application/postscript";
		mimeTypes[".rtf"] = "application/rtf";
		mimeTypes[".xls"] = "application/vnd.ms-excel";
		mimeTypes[".ppt"] = "application/vnd.ms-powerpoint";
		mimeTypes[".wmlc"] = "application/vnd.wap.wmlc";
		mimeTypes[".kml"] = "application/vnd.google-earth.kml+xml";
		mimeTypes[".kmz"] = "application/vnd.google-earth.kmz";
		mimeTypes[".7z"] = "application/x-7z-compressed";
		mimeTypes[".cco"] = "application/x-cocoa";
		mimeTypes[".jardiff"] = "application/x-java-archive-diff";
		mimeTypes[".jnlp"] = "application/x-java-jnlp-file";
		mimeTypes[".run"] = "application/x-makeself";
		mimeTypes[".pl"] = "application/x-perl";
		mimeTypes[".pm"] = "application/x-perl";
		mimeTypes[".prc"] = "application/x-pilot";
		mimeTypes[".pdb"] = "application/x-pilot";
		mimeTypes[".rar"] = "application/x-rar-compressed";
		mimeTypes[".rpm"] = "application/x-redhat-package-manager";
		mimeTypes[".sea"] = "application/x-sea";
		mimeTypes[".swf"] = "application/x-shockwave-flash";
		mimeTypes[".sit"] = "application/x-stuffit";
		mimeTypes[".tcl"] = "application/x-tcl";
		mimeTypes[".tk"] = "application/x-tcl";
		mimeTypes[".der"] = "application/x-x509-ca-cert";
		mimeTypes[".pem"] = "application/x-x509-ca-cert";
		mimeTypes[".crt"] = "application/x-x509-ca-cert";
		mimeTypes[".xpi"] = "application/x-xpinstall";
		mimeTypes[".xhtml"] = "application/xhtml+xml";
		mimeTypes[".zip"] = "application/zip";
		mimeTypes[".bin"] = "application/octet-stream";
		mimeTypes[".exe"] = "application/octet-stream";
		mimeTypes[".dll"] = "application/octet-stream";
		mimeTypes[".deb"] = "application/octet-stream";
		mimeTypes[".dmg"] = "application/octet-stream";
		mimeTypes[".eot"] = "application/octet-stream";
		mimeTypes[".iso"] = "application/octet-stream";
		mimeTypes[".img"] = "application/octet-stream";
		mimeTypes[".msi"] = "application/octet-stream";
		mimeTypes[".msp"] = "application/octet-stream";
		mimeTypes[".msm"] = "application/octet-stream";
		mimeTypes[".mid"] = "audio/midi";
		mimeTypes[".midi"] = "audio/midi";
		mimeTypes[".kar"] = "audio/midi";
		mimeTypes[".mp3"] = "audio/mpeg";
		mimeTypes[".ogg"] = "audio/ogg";
		mimeTypes[".m4a"] = "audio/x-m4a";
		mimeTypes[".ra"] = "audio/x-realaudio";
		mimeTypes[".3gpp"] = "video/3gpp";
		mimeTypes[".3gp"] = "video/3gpp";
		mimeTypes[".mp4"] = "video/mp4";
		mimeTypes[".mpeg"] = "video/mpeg";
		mimeTypes[".mpg"] = "video/mpeg";
		mimeTypes[".mov"] = "video/quicktime";
		mimeTypes[".webm"] = "video/webm";
		mimeTypes[".flv"] = "video/x-flv";
		mimeTypes[".m4v"] = "video/x-m4v";
		mimeTypes[".mng"] = "video/x-mng";
		mimeTypes[".asx"] = "video/x-ms-asf";
		mimeTypes[".asf"] = "video/x-ms-asf";
		mimeTypes[".wmv"] = "video/x-ms-wmv";
		mimeTypes[".avi"] = "video/x-msvideo";
	}
	std::map<std::string, std::string>::const_iterator iterator = mimeTypes.find(extension);
	if (iterator != mimeTypes.end())
		return (iterator->second);
	return ("application/octet-stream");
}