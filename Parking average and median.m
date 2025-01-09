channelID = 2802930;
readAPIKey = '87FSAYRWUUKYUTTD';
url = ['https://api.thingspeak.com/channels/', num2str(channelID), '/feeds.json'];

try
    response = webread(url, 'api_key', readAPIKey);
catch ME
    disp('Error fetching data from ThingSpeak:');
    disp(ME.message);
    return;
end

field4_data = [];
for i = 1:length(response.feeds)
    field4_data = [field4_data, str2double(response.feeds(i).field4)];
end

field4_data = field4_data(~isnan(field4_data));

if isempty(field4_data)
    disp('No valid data available for analysis.');
    return;
end

avg_parking_distance = mean(field4_data);
median_parking_distance = median(field4_data);

plot(field4_data, 'b-', 'LineWidth', 2);
hold on;
yline(avg_parking_distance, 'r--', ['Avg: ', num2str(avg_parking_distance, '%.2f')], 'LineWidth', 2);
yline(median_parking_distance, 'g--', ['Median: ', num2str(median_parking_distance, '%.2f')], 'LineWidth', 2);
title('Parking Distance Data');
xlabel('Data Points');
ylabel('Distance (cm)');
legend('Parking Distance', 'Average', 'Median');
grid on;
hold off;
