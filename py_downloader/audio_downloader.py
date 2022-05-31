from pytube import YouTube

print("========== Welcome to the SRI YouTube Downloader ==========")
while True:
    try:
        link = input("Provide a YouTube link: ")
        yt = YouTube(link)
    except:
        print("ERROR: Invalid link provided. Please try again.")
    else:
        break

print("\n========== Getting information about your download... ==========")
print("Title:", yt.title)
print("Number of views:", yt.views)
print("Length of video:", yt.length, "seconds")
print("Description:", yt.description)

print("\n========== Getting all available streams... ==========")
sound = yt.streams.filter(only_audio=True)
print(sound)
sound[0].download(filename="speech.wav")