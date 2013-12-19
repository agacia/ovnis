require 'rubygems'
require 'gnuplot' 
require 'optparse'

# edge_id x y
def readEdges(file_name)
    text=File.open(file_name).read
    data = Hash.new
    text.each_line { |line|
        record = line.split("\t")
        edge_id = record[0]
        x = record[1].to_f
        y = record[2].to_f
        data[edge_id] = [x,y]
    }
    puts "read edges: #{data.length}"
    return data
end
            
# route_id edge_id,length,max_speed ...
def readRoutes(file_name)
    text = File.open(file_name).read
    routes = Hash.new
    max_travelTime = 0
    text.each_line { |line|
        record = line.split("\t")
        route_id = record[0]
        travelTime = 0
        routes[route_id] = Array.new
        for i in 1..record.length-1 
            route_details = record[i].split(",")
            edge_id = route_details[0]
            edge_length = route_details[1].to_f
            edge_max_speed = route_details[2].to_f
            edge_static_cost = edge_length / edge_max_speed
            routes[route_id] << [edge_id,edge_length,edge_max_speed,edge_static_cost]
            travelTime += edge_static_cost 
            #puts "#{route_id}: #{routes[route_id]}"
        end
        if travelTime > max_travelTime then
            max_travelTime = travelTime
        end
    }
    puts "read routes: #{routes.length}"
    return [max_travelTime, routes]
end
        
def compute_distance(x1, y1, x2, y2)
    dx = x2 - x1 #horizontal difference
    dy = y2 - y1 #vertical difference
    distance = Math.sqrt(dx * dx + dy * dy)
    return distance
end
   
def routeContainsEdgeExcludeMargins?(routes, route_id, edgeid, start_edge_id, end_edge_id) 
    isMonitored = false;
    routes[route_id].each { |edge|
        edge_id = edge[0]
        if edge_id == end_edge_id then
            isMonitored = false
        end
        if isMonitored then
            if edge_id == edgeid then
                return true
            end
        end
        if edge_id == start_edge_id then
            isMonitored = true
        end
    }
    return false
end

def numberOfEdgesInRouteExcludeMargins(routes, route_id, start_edge_id, end_edge_id) 
    isMonitored = false;
    number_of_edges = 0
    routes[route_id].each { |edge|
        edge_id = edge[0]
        if edge_id == end_edge_id then
            isMonitored = false
        end
        if isMonitored then
            number_of_edges += 1
        end
        if edge_id == start_edge_id then
            isMonitored = true
        end
    }
    return number_of_edges
end


def readVanetsKnowledge(dir_name, output_file_name, file_name, routes, edges, warmingTime, distance_granulaiton, outputCandlestickFile)
	if routes == nil || routes.empty? || edges == nil || edges.empty? then
		puts "Missing routes or edges"
        return 
	end
    
    coverage_write = File.open("#{dir_name}#{output_file_name}_coverage", 'w')
    quality_write = File.open("#{dir_name}#{output_file_name}_quality", 'w')
    age_write = File.open("#{dir_name}#{output_file_name}_age", 'w')
    
    text=File.open(file_name).read
    knowledge = Array.new
    informationAge = Hash.new
    coverage = Hash.new
    total_max_distance = 0
    sum_coverage = 0
    count_coverage = 0
    
    text = File.open(file_name).read
    
    puts "read #{text.length} lines"
	text.each_line { |line|

        record = line.split("\t")
        dateStamp = record[i=0].to_f
        if dateStamp > warmingTime then
            x = 0
            if record.size > 3 then
                x = record[i+=1].to_f
                y = record[i+=1].to_f
                x = 1000 - x
                y = y + 40
                decision_edge = record[i+=1]
                destination_edge = record[i+=1]
            end
            max_distance = 0
            for j in (i+1)..(record.length-routes.length-2)
                number_of_edges = 0
                if record[j].strip!.nil? then
                    edge_info = record[j].split(",")
                    edge_id = edge_info[0]
                    number_of_edges += 1
                    i += 1
                    if record.size > 3 then
                        distance = compute_distance(x,y,edges[edge_id][0],edges[edge_id][1])
                        if distance > 3000 then
                            puts "#{x},#{y} to #{edges[edge_id][0]},#{edges[edge_id][1]}= #{distance} no!"
                        end
                    end
                    if distance > max_distance then
                        max_distance = distance
                    end
                    if distance > total_max_distance then
                        total_max_distance = distance
                    end
                    age = dateStamp - edge_info[1].to_f
                    distance_index = (distance / distance_granulaiton).floor
                    if not informationAge.has_key?(distance_index) then
                        informationAge[distance_index] = {"count" => 1, "min_age" => age, "max_age" => age, "avg_age" => age, "values"=>Array.new, "first_quartile"=>0, "median"=>0, "third_quartile"=>0, "distance"=>"distance"}
                    else
                        count = informationAge[distance_index]["count"]
                        informationAge[distance_index]["max_age"] = age > informationAge[distance_index]["max_age"] ? age : informationAge[distance_index]["max_age"]
                        informationAge[distance_index]["min_age"] = age < informationAge[distance_index]["min_age"] ? age : informationAge[distance_index]["min_age"]
                        informationAge[distance_index]["avg_age"] = (informationAge[distance_index]["avg_age"] * count + age) / (count + 1)
                        informationAge[distance_index]["values"].push(age)
                        informationAge[distance_index]["count"] = count + 1
                    end
                end
            end
            
            number_of_edges = 0
            route_number_of_edges = 0
            number_of_edges_coverage = 0
            for j in (i+1)..record.length-1
                if record[j].strip!.nil? then
                    number_of_edges_info = record[j].split(",")
                    route_id = number_of_edges_info[0]
                    number_of_edges += number_of_edges_info[1].to_f
                    number_of_edges_on_route = numberOfEdgesInRouteExcludeMargins(routes, route_id, decision_edge, destination_edge)
                    route_number_of_edges += number_of_edges_on_route
                    #puts "route_id: #{route_id}:#{number_of_edges_info[1]}/#{number_of_edges_on_route}"
                end
            end
            if route_number_of_edges != 0 then
                number_of_edges_coverage = (number_of_edges / route_number_of_edges).round(2)
                sum_coverage += number_of_edges
                if number_of_edges != 0 then
                    count_coverage += route_number_of_edges
                end
            end
            
            knowledge.push({"dateStamp"=>dateStamp, "number_of_edges_coverage"=>number_of_edges_coverage, "distance_range"=>max_distance})
            coverage_write.puts "#{dateStamp.round(2)} \t #{route_id} \t #{number_of_edges_coverage} \t #{number_of_edges.round} \t #{route_number_of_edges} \t #{max_distance.round(2)}"
             
        end
    }
    
    avg_coverage = count_coverage == 0 ? 0 : (sum_coverage / count_coverage).round(2)
    puts "count of non zero knowledge: #{sum_coverage} avg_coverage: #{avg_coverage}"
    quality_write.puts "count of non zero knowledge: #{sum_coverage} avg_coverage: #{avg_coverage}"
    
    ages = Hash.new
    ages[:distance] = Array.new
    ages[:min] = Array.new
    ages[:first_quartile] = Array.new
    ages[:median] = Array.new
    ages[:third_quartile] = Array.new
    ages[:max] = Array.new
    ages[:mean] = Array.new
    ages[:count] = Array.new
    
    for i in 0..total_max_distance/distance_granulaiton
        if informationAge.has_key?(i)
            distance_min = i * distance_granulaiton
            distance_max = distance_min + distance_granulaiton
            min_age = informationAge[i]["min_age"]
            max_age = informationAge[i]["max_age"]
            avg_age =  informationAge[i]["avg_age"]
            count = informationAge[i]["count"]
            agesSort = informationAge[i]["values"].sort
            informationAge[i]["distance"] = "#{distance_min}-#{distance_max}"
            informationAge[i]["first_quartile"] = agesSort[agesSort.size/4].round(2)
            informationAge[i]["median"] = agesSort[agesSort.size/2].round(2)
            informationAge[i]["third_quartile"] = agesSort[agesSort.size/4*3].round(2)
            age_write.puts "#{distance_min}-#{distance_max} \t #{min_age.round(2)} \t #{informationAge[i]['first_quartile']} \t #{informationAge[i]['median']} \t #{informationAge[i]['third_quartile']} \t #{max_age.round(2)} \t #{avg_age.round(2)} \t #{count}"
            ages[:distance].push(distance_max)
            ages[:min].push(min_age)
            ages[:first_quartile].push(informationAge[i]["first_quartile"])
            ages[:median].push(informationAge[i]["median"])
            ages[:third_quartile].push(informationAge[i]["third_quartile"])
            ages[:max].push(max_age)
            ages[:mean].push(avg_age)
            ages[:count].push(count)
        end
    end
    
    Gnuplot.open { |gp|
        
        Gnuplot::Plot.new( gp ) { |plot|
            plot.term('pdf')	
            #plot.title  "Information age at distance"
            plot.ylabel "Age"
            plot.xlabel "Distance"
            plot.xrange "[#{ages[:distance].min-distance_granulaiton}:#{ages[:distance].max+distance_granulaiton}]"
            plot.yrange "[#{ages[:min].min-3}:#{ages[:max].max+3}]"
            plot.set("boxwidth", "100 absolute")
            plot.set("key font"," \",10\"")
            #plot.set("key samplen", "20")
            plot.set("key spacing", "3")
            plot.set('grid ytics','lt 3 lw 1 lc rgb "#000000"')
            plot.set('xlabel font ', " \",10\"" )
            plot.set('ylabel font ', " \",10\"" )
            plot.set('xtics font ', " \",10\"" )
            plot.set('ytics font ', " \",10\"" )

            #plot.set("style" "fill empty")
            plot.data << Gnuplot::DataSet.new( [ages[:distance],ages[:min],ages[:first_quartile],ages[:median],ages[:third_quartile],ages[:max],ages[:mean]] ) do |ds|
                ds.using = "1:3:2:6:5"
                ds.with = "candlesticks lt 3 lw 2 title 'Min, max' whiskerbars"
                # ds.title = "Quartiles"
            end
            plot.data << Gnuplot::DataSet.new( [ages[:distance],ages[:min],ages[:first_quartile],ages[:median],ages[:third_quartile],ages[:max],ages[:mean]] ) do |ds|
                ds.using = "1:3:3:5:5"
                ds.with = "candlesticks lt 3 lw 2 lc 4 title '1st, 3rd Quartiles' whiskerbars"
                # ds.title = "Quartiles"
            end
            #median - first stroke/line/whisker
            plot.data << Gnuplot::DataSet.new( [ages[:distance],ages[:min],ages[:first_quartile],ages[:median],ages[:third_quartile],ages[:max],ages[:mean]] ) do |ds|
                ds.using = "1:4:4:4:4"
                ds.with = "candlesticks lt 1 lc 3 lw 1"
                ds.title = "Median"
                #ds.title = "median"
            end   
            #mean - secod stroke
            plot.data << Gnuplot::DataSet.new( [ages[:distance],ages[:min],ages[:first_quartile],ages[:median],ages[:third_quartile],ages[:max],ages[:mean]] ) do |ds|
                ds.using = "1:7:7:7:7"
                ds.with = "candlesticks lt -1 lc 2 lw 1"
                # ds.notitle
                ds.title = "Average"
            end  
            
            plot.output(outputCandlestickFile)	
        }  
        
    }

    
end

options = {}
OptionParser.new do |opts|
    opts.banner = "Usage: ruby filename [options]"
    opts.on('-d', '--directory DIRECTORY', 'directory name') { |v| options[:directory] = v }
    opts.on('-k', '--knowledge KNOWLEDGE_FILE', 'Input file name') { |v| options[:knowledge_file] = v }
    opts.on('-e', '--edges EDGES_FILE', 'Input file name') { |v| options[:edges_file] = v }
    opts.on('-r', '--routes ROUTES_FILE', 'Input file name') { |v| options[:routes_file] = v }
    opts.on('-o', '--output OUTPUT', 'Output dir name') { |v| options[:output] = v }
end.parse!

puts options

directory = options.has_key?(:directory) ? options[:directory] : ""
knowledge_file = options.has_key?(:knowledge_file) ? options[:knowledge_file] : "vanets_knowledge"
edges_file = options.has_key?(:edges_file) ? options[:edges_file] : "edges_positions"
routes_file = options.has_key?(:routes_file) ? options[:routes_file] : "routes_info"
output = options.has_key?(:output) ? options[:output] : "data_quality"

edges = readEdges("#{directory}#{edges_file}")
routes_result = readRoutes("#{directory}#{routes_file}")
outputCandlestickFile = "#{directory}age_candlestick.pdf"
readVanetsKnowledge(directory, output, "#{directory}#{knowledge_file}", routes_result[1], edges, 0, 500, outputCandlestickFile)

