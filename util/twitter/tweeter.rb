#!/usr/bin/ruby

require "rubygems"
require "twitter"

if ARGV.length < 9
	$stderr.puts "Usage: #{$0} name role race gender align deathlev hp maxhp death..."
	exit(false)
end

name = ARGV[0]
role = ARGV[1]
race = ARGV[2]
gender = ARGV[3]
align = ARGV[4]
deathlev = ARGV[5]
hp = ARGV[6]
maxhp = ARGV[7]
death = ARGV[8]

Twitter.configure do |config|
	config.consumer_key = "FbCxMB6lEcqFLOAwP9z0Sg"
	config.consumer_secret = ""
	config.oauth_token = "383855529-7uvamZJm3l7JSvVY9PaUyiOFwAnlnVcmmbDSNcfR"
	config.oauth_token_secret = ""
end

client = Twitter::Client.new

client.update("#{name}-#{role}-#{race}-#{gender}-#{align} #{death} on level #{deathlev} (HP: #{hp} [#{maxhp}])")
