#!/usr/bin/env ruby

# update-symbols-header.rb
#   Regenerate the header file for the symbols font.
#
# Usage:
#   bin/update-symbols-header.rb version [icons.yml] > engine/Util/Symbol.h
#
#   version - The FontAwesome version (e.g. "4.2.0").
#   icons.yml - (Optional) FontAwesome icons.yml file.
#               If not specified, the file will be fetched from GitHub.

require 'yaml'
require 'open-uri'

version, filename = *ARGV

if !version
	puts "Usage: #{$0} version [filename]"
	exit false
end

class FaCategory

	def initialize(name)
		@name = name
		@symbols = []
	end

	def load_symbol(obj)
		unicode = obj['unicode']

		@symbols.push(FaSymbol.new(obj['id'], unicode))
		if obj['aliases']
			obj['aliases'].each do |symalias|
				@symbols.push(FaSymbol.new(symalias, unicode))
			end
		end
	end

	def draw
		puts "\t//{{{ #{@name}"
		@symbols.each { |symbol| symbol.draw }
		puts "\t//}}} #{@name}"
		puts
	end
end

class FaSymbol

	def initialize(id, unicode)
		@id = ident(id)
		@unicode = unicode
	end

	def ident(s)
		s.upcase.gsub(/[^A-Z0-9]/, '_')
	end

	def draw
		puts "\t#{@id} = 0x#{@unicode},"
	end
end

def load_syms(icons)
	retv = Hash.new { |hash, key| hash[key] = FaCategory.new(key) }

	icons.each do |icon|
		retv[icon['categories'][0]].load_symbol(icon)
	end

	retv
end

if filename
	yaml = YAML.load_file(filename)
else
	url = "https://raw.githubusercontent.com/FortAwesome/Font-Awesome/v#{version}/src/icons.yml"
	STDERR.puts "Loading from #{url}"
	open(url) do |io|
		yaml = YAML.load(io)
	end
end

all_syms = load_syms(yaml['icons'])

puts <<EOD
// Symbol.h
//
// Copyright (c) #{Time.now.year} Michael Imamura.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which
// you have taken this file. If you can not find the license you can not use
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions
// and limitations under the License.

#pragma once

namespace HoverRace {
namespace Util {

namespace Symbol {
enum {
EOD

all_syms.keys.sort.each do |category|
	all_syms[category].draw
end

puts <<EOD
};
}  // namespace Symbol

}  // namespace Util
}  // namespace HoverRace
EOD
