#!/usr/bin/ruby

require "rubygems"
require "twitter"

if ARGV.length < 9
	$stderr.puts "Usage: " + $0 + " name role race gender align deathlev hp maxhp death..."
	exit(false)
end

playlog = {}

playlog["name"] = ARGV[0]
playlog["role"] = ARGV[1]
playlog["race"] = ARGV[2]
playlog["gender"] = ARGV[3]
playlog["align"] = ARGV[4]
playlog["deathlev"] = ARGV[5]
playlog["hp"] = ARGV[6]
playlog["maxhp"] = ARGV[7]
playlog["death"] = ARGV.slice(8, ARGV.length - 8).join(" ")

Twitter.configure do |config|
	config.consumer_key = "FbCxMB6lEcqFLOAwP9z0Sg"
	config.consumer_secret = ""
	config.oauth_token = "383855529-7uvamZJm3l7JSvVY9PaUyiOFwAnlnVcmmbDSNcfR"
	config.oauth_token_secret = ""
end

client = Twitter::Client.new

client.update("#{playlog['name']}-#{playlog['role']}-#{playlog['race']}-#{playlog['gender']}-#{playlog['align']} #{playlog['death']} on level #{playlog['deathlev']} (HP: #{playlog['hp']} [#{playlog['maxhp']}])")
