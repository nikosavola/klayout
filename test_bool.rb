ly = RBA::Layout::new
ly.dbu = 0.001
top = ly.add_cell("TOP")

l1 = ly.layer(1, 0)
l2 = ly.layer(2, 0)

n = 1000
puts "Generating data..."
n.times do |x|
  n.times do |y|
    ly.cell(top).shapes(l1).insert(RBA::Box::new(x*10, y*10, x*10+8, y*10+8))
    ly.cell(top).shapes(l2).insert(RBA::Box::new(x*10+4, y*10+4, x*10+12, y*10+12))
  end
end

puts "Running boolean..."
ep = RBA::EdgeProcessor::new
# Need to use Region
reg1 = RBA::Region::new(ly.cell(top).shapes(l1))
reg2 = RBA::Region::new(ly.cell(top).shapes(l2))

r = reg1 & reg2
puts "Result has #{r.size} polygons"
