from bs4 import BeautifulSoup
import requests, urllib, json, sys

class Parser:
    def __init__(self):
        self.groups = None
        self.target_group = None
        self.lessons = None

    def get_groups(self):
        json_obj = None
        url = 'https://ruz.spbstu.ru/faculty/125/groups'
        html = requests.get(url).text
        soup = BeautifulSoup(html, "html.parser")
        scripts = soup.findAll('script')
        for script in scripts:
            if 'window.__INITIAL_STATE__' in script.text:
                json_string = script.text.replace('window.__INITIAL_STATE__ =', '').replace('None', '')
                json_string = json_string.strip()[:-1]
                json_obj = json.loads(json_string)
                break

        self.groups = json_obj

    def get_group(self, target_group):
        for group_obj in self.groups['groups']['data']['125']:
            if group_obj['name'] == target_group:
                self.target_group = group_obj

    def get_lessons_by_date(self, date):
        url = f'https://ruz.spbstu.ru/api/v1/ruz/scheduler/{self.target_group["id"]}?date={date}'
        print(url)
        data = requests.get(url).text
        self.lessons = json.loads(data)
        
    def print_lessons_at_date(self, date):
        self.get_lessons_by_date(date)
        if self.lessons == None: print(f"Cannot get lessons at {date}")
        else:
            print(f"Week: {self.lessons['week']['date_start']} - {self.lessons['week']['date_end']}")
            print(f"Date: {self.lessons['days'][0]['date']}")
            for lesson in self.lessons['days'][0]['lessons']:
                print(lesson['subject'])
                print(lesson['teachers'][0]['full_name'])
                print(f"Auditorie: {lesson['auditories'][0]['name']} {lesson['auditories'][0]['building']['name']}")
                print(f"Time: {lesson['time_start']} - {lesson['time_end']}")
                print()

def format_date(date: str) -> str:
    splitted = date.split('.')
    return f"{int(splitted[0])}-{int(splitted[1])}-{int(splitted[2])}"

def main():
    parser = Parser()

    target_group = '5151003/30002'
    date = '2024.09.17'

    parser.get_groups()
    if parser.groups == None: sys.exit('Cannot get groups')
    parser.get_group(target_group)
    if parser.target_group == None: sys.exit('Cannot get target group')

    new_date = format_date(date)
    
    parser.print_lessons_at_date(new_date)
    
    

if __name__ == "__main__":
    main()
