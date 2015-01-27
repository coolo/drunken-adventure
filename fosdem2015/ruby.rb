require 'date'

f = File.new('factory.csv')
f.readlines().each do |line|
  s=line.split(',')
  number=s[1].to_i
  year, week =s[0].split('-')
#  print "#{number} #{year} #{week}\n"
  d = Date.new(year.to_i, 1, 1)
  d += week.to_i * 7
  puts "#{d},#{number}"
  
end
