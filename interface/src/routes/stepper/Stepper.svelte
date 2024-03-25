<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { notifications } from '$lib/components/toasts/notifications';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Stepper from '~icons/tabler/rotate';
	import Info from '~icons/tabler/info-circle';
	import Save from '~icons/tabler/device-floppy';
	import Stop from '~icons/tabler/hand-stop';

	type StepperControl = {
		isEnabled: boolean;
		direction: boolean;
		speed: number;
		acceleration: number;
		current: number;
	};

	let stepperControl: StepperControl = { isEnabled: false, direction: false, speed: 128, acceleration: 30, current: 800 };

	let lightOn = false;
	let lightBrightness = 128;

	const ws_token = $page.data.features.security ? '?access_token=' + $user.bearer_token : '';

	const stepperControlSocket = new WebSocket('ws://' + $page.url.host + '/ws/stepperControl' + ws_token);

	stepperControlSocket.onopen = (event) => {
		stepperControlSocket.send('Hello');
	};

	stepperControlSocket.addEventListener('close', (event) => {
		const closeCode = event.code;
		const closeReason = event.reason;
		console.log('WebSocket closed with code:', closeCode);
		console.log('Close reason:', closeReason);
		notifications.error('Websocket disconnected', 5000);
	});

	stepperControlSocket.onmessage = (event) => {
		const message = JSON.parse(event.data);
		if (message.type != 'id') {
			stepperControl = message;
		}
	};

	onDestroy(() => stepperControlSocket.close());

</script>

<SettingsCard collapsible={false}>
	<Stepper slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">Stepper</span>
	<div class="w-full">
		<!-- <h1 class="text-xl font-semibold">Websocket Example</h1>
		<div class="alert alert-info my-2 shadow-lg">
			<Info class="h-6 w-6 flex-shrink-0 stroke-current" />
			<span
				>The switch below controls the LED via the WebSocket. It will automatically update whenever
				the LED state changes.</span
			>
		</div> -->
		<div class="form-control w-52">
			<label class="label cursor-pointer">
				<span class="">Enable</span>
				<input
					type="checkbox"
					class="toggle toggle-primary"
					bind:checked={stepperControl.isEnabled}
					on:change={() => {
						stepperControlSocket.send(JSON.stringify(stepperControl));
					}}
				/>
			</label>
		</div>
		<div class="form-control w-52">
			<label class="label cursor-pointer">
				<span class="">Direction</span>
				<input
					type="checkbox"
					class="toggle toggle-primary"
					bind:checked={stepperControl.direction}
					on:change={() => {
						stepperControlSocket.send(JSON.stringify(stepperControl));
					}}
				/>
			</label>
		</div>
		<div class="form-control">
			<label class="label cursor-pointer">
				<span class="mr-4">Speed </span>
				<input 
					type="range"
					min="0" 
					max="400" 
					class="range range-primary"
					bind:value={stepperControl.speed}
					on:change={() => {
						stepperControlSocket.send(JSON.stringify(stepperControl));
					}}
				/>
			</label>
		</div>

		<div class="form-control">
			<label class="label cursor-pointer">
				<span class="mr-4">Acceleration </span>
				<input 
					type="range"
					min="0" 
					max="200" 
					class="range range-primary"
					bind:value={stepperControl.acceleration}
					on:change={() => {
						stepperControlSocket.send(JSON.stringify(stepperControl));
					}}
				/>
			</label>
		</div>
		<div class="form-control">
			<label class="label cursor-pointer">
				<span class="mr-4">Current </span>
				<input 
					type="range"
					min="0" 
					max="4000" 
					class="range range-primary"
					bind:value={stepperControl.current}
					on:change={() => {
						stepperControlSocket.send(JSON.stringify(stepperControl));
					}}
				/>
			</label>
		</div>
		<div class="flex flex-row flex-wrap justify-between gap-x-2">
			<div class="flex-grow"></div>
			<div>
				<div>
					<button class="btn btn-primary inline-flex items-center" on:click={() => {stepperControl.speed=0; stepperControlSocket.send(JSON.stringify(stepperControl));}}
						><Stop class="mr-2 h-5 w-5" /><span>Stop</span></button
					>
				</div>
			</div>
		</div>
	</div>
</SettingsCard>
