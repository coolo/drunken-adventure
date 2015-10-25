class Warrior < ActiveRecord::Base
   belongs_to :user
   belongs_to :war

   has_many :estimates

   def averages
     @averages || calc_averages
   end

   def average(base)
     averages[base]
   end
   
   private
   
   def calc_averages
     @averages = []
     sums = Array.new(war.count, 0.0)
     count = Array.new(war.count, 0)
     Rails.logger.debug "#{user.name}: #{estimates.map { |e| [e.user_id, e.base, e.stars] }.inspect}"
     estimates.each do |e|
       sums[e.base-1] += e.stars
       count[e.base-1] += 1
     end
     war.count.times do |i|
       if count[i] > 0
         @averages[i] = (sums[i] / count[i] * 10).to_i / 10.0
       else
         @averages[i] = 0
       end
     end
     @averages
   end
end
