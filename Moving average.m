
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

window_size = 5;
moving_avg = movmean(field4_data, window_size);

plot(moving_avg, 'm-', 'LineWidth', 2);
title('Moving Average of Parking Distance');
xlabel('Data Points');
ylabel('Moving Average (cm)');
grid on;
